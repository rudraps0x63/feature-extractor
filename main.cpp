#include "libs/types.h"
#include "libs/chunker.h"
#include "libs/callback-sink.h"
#include "libs/fex.h"
#include "libs/logger.h"

#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>

using namespace Logger;

/* Global entities */
std::mutex g_mu;
PCMDataQueue g_dataQueue;
std::atomic_bool g_chunkerStopped = false;


static void
writeToFeatureFile(const std::vector<std::vector<std::vector<float>>>& data)
{
    static unsigned idx = 0;
    std::string fileName = "data/features_" + std::to_string(idx++) + ".bin";
    std::ofstream out { fileName, std::ios::out | std::ios::binary | std::ios::trunc };

    if (!out)
        Log(Level::WARN) << "Cannot open for writing!\n";

    for (const auto &dataInner : data) {
        for (const auto &row : dataInner)
            out.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(float));
    }
}


static void
marsBufferListCb(GstBufferList* buffers,
                 gpointer       user_data)
{
    PCMBytesVec PCMBytesVector;
    std::streamsize sizeInBytes = 0;
    uint len = gst_buffer_list_length(buffers);

    std::string msg { "Got buffer list with: " + std::to_string(len) + " buffers.\n" };
    Log() << msg;

    for (uint i = 0; i < len; ++i) {
        GstBuffer* buffer = gst_buffer_list_get(buffers, i);
        GstMapInfo mapInfo;

        if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
            std::string msg { "Couldn't map buffer #" + std::to_string(i + 1) + "! Continuing...\n" };
            Log(Level::WARN) << msg;
            continue;
        }

        std::string msg { "Got " + std::to_string(mapInfo.size)
                          + " valid bytes for buffer #" + std::to_string(i + 1) + " (max bytes: "
                          + std::to_string(mapInfo.maxsize) + ").\n" };
        Log() << msg;

        /**
         * Push each valid byte from all the buffers consecutively. We will
         * use this vector later on to extract features.
        */
        for (uint byteOffset = 0; byteOffset < mapInfo.size; ++byteOffset) {
            uint8_t bufferByte = *(mapInfo.data + byteOffset);

            PCMBytesVector.push_back(bufferByte);
            ++sizeInBytes;
        }

        gst_buffer_unmap(buffer, &mapInfo);
    }

    /**
     * TODO: Optimize --
     * 1) Create the object first and update it instead of pushing a copy
     * 2) If pushing too quickly to the queue, wait until a certain threshold has been consumed
    */
    {
        g_mu.lock();

        g_dataQueue.emplace(std::pair{ PCMBytesVector, sizeInBytes });

        g_mu.unlock();
    }

    std::cout << '\n';
}


static void
launchChunker()
{
    GstElement* sink = nullptr;
    MarsChunker* chunker = nullptr;

    sink = mars_callback_sink_new();
    mars_callback_sink_set_buffer_list_callback(MARS_CALLBACK_SINK(sink), marsBufferListCb, nullptr, nullptr);

    chunker = mars_chunker_new_with_sink(MARS_CHUNKER_INPUT_MIC, "audioresample", sink);
    mars_chunker_set_maximum_chunk_time(chunker, 2 * GST_SECOND);

    mars_chunker_play(chunker);

    if (!mars_chunker_is_playing(chunker))
        throw std::runtime_error("Couldn't launch chunker, abort.");

    std::string msg { "Chunker launched. Input given: " + std::string(mars_chunker_get_input(chunker)) + '\n' };
    Log() << msg;

    /* Let the chunker play */
    std::cin.get();

    mars_chunker_stop(chunker);
    g_chunkerStopped = true;
}


void
launchFeatureExtractor()
{
    PCMData data;
    FeatureExtractor fex;

    while (true) {
        if (g_dataQueue.empty()) {
            if (g_chunkerStopped)
                return;

            continue;
        }

        std::string msg { "Queue size = " + std::to_string(g_dataQueue.size()) + ". Processing...\n" };
        Log() << msg;

        {
            g_mu.lock();

            data = g_dataQueue.front();
            g_dataQueue.pop();

            g_mu.unlock();
        }

        writeToFeatureFile(fex.run(data));
    }
}


int
main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    std::thread t1(launchChunker);
    std::thread t2(launchFeatureExtractor);

    t1.join();
    t2.join();

    return 0;
}
