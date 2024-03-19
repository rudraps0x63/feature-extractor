DEPENDS := glib-2.0 gstreamer-1.0 gstreamer-base-1.0

CC := g++
# $(...) is also used to evaluate Makefile variables, so choose `...` or $$(...) instead for shell cmds
CC_FLAGS := -c -fpermissive `pkg-config --cflags ${DEPENDS}`
LD_FLAGS := `pkg-config --libs ${DEPENDS}`

LIB_DIR := libs
BUILD_DIR := _build
SOURCES := main.cpp ${wildcard ${LIB_DIR}/*.c}
EXEC := fex

all: builddir bin

builddir:
	@mkdir -p ${BUILD_DIR}

john:
	echo ${wildcard ${BUILD_DIR}/*.o}

bin: main.o callback-sink.o chunker.o
	@${CC} ${LD_FLAGS} ${wildcard ${BUILD_DIR}/*.o} -o ${EXEC}
	@echo "make successful! Built binary: fex."
	@echo "Run with ./fex or add the current working directory to PATH envvar and execute it as a normal program."

main.o: main.cpp
	@${CC} ${CC_FLAGS} main.cpp -o ${BUILD_DIR}/main.o

callback-sink.o: ${LIB_DIR}/callback-sink.c
	@${CC} ${CC_FLAGS} ${LIB_DIR}/callback-sink.c -o ${BUILD_DIR}/callback-sink.o

chunker.o: ${LIB_DIR}/chunker.c
	@${CC} ${CC_FLAGS} ${LIB_DIR}/chunker.c -o ${BUILD_DIR}/chunker.o

clean:
	@rm -rf ${wildcard ./*.o ${EXEC} ${BUILD_DIR}}


# # Makefile for building the audio player

# CXX := g++

# CXXFLAGS := -g -fpermissive $(shell pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0)

# LDFLAGS := $(shell pkg-config --libs gstreamer-1.0 gstreamer-base-1.0)

# SOURCES := libs/*.c main.cpp

# BUILD_DIR := build

# # Data directory for .wav/bin files
# DATA_DIR := data

# # Object files - now including the build directory
# OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# # Executable name
# EXECUTABLE := fex

# .PHONY: all clean

# all: $(BUILD_DIR)/$(EXECUTABLE)

# $(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS)
# 	@mkdir -p $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# # Pattern rule for object files, now placing them in the build directory
# $(BUILD_DIR)/%.o: %.cpp
# 	@mkdir -p $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(EXECUTABLE) $(BUILD_DIR)/$(DATA_DIR)/* $(HOME)/$(DATA_DIR)/*.wavw ${HOME}/${DATA_DIR}/*.bin
