#pragma once

#include <vector>
#include <tuple>
#include <queue>
#include <cstdint>
#include <ios>

using PCMBytesVec = std::vector<uint8_t>;
using PCMData = std::pair<PCMBytesVec, std::streamsize>;
using PCMDataQueue = std::queue<PCMData>;