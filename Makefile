DEPENDS := glib-2.0 gstreamer-1.0 gstreamer-base-1.0

CC := g++
CC_FLAGS := -c -fpermissive `pkg-config --cflags ${DEPENDS}`
LD_FLAGS := `pkg-config --libs ${DEPENDS}`

LIB_DIR := libs
BUILD_DIR := _build
DATA_DIR := data
SOURCES := main.cpp ${wildcard ${LIB_DIR}/*.c}
EXEC := fex

all: builddir bin

builddir:
	@mkdir -p ${BUILD_DIR}

bin: main.o callback-sink.o chunker.o fex.o
	@${CC} ${LD_FLAGS} ${wildcard ${BUILD_DIR}/*.o} -o ${EXEC}
	@echo "make successful! Built binary: fex."
	@echo "Run with ./fex or add the current working directory to PATH envvar and execute it as a normal program."

main.o: main.cpp
	@${CC} ${CC_FLAGS} main.cpp -o ${BUILD_DIR}/main.o

callback-sink.o: ${LIB_DIR}/callback-sink.c
	@${CC} ${CC_FLAGS} ${LIB_DIR}/callback-sink.c -o ${BUILD_DIR}/callback-sink.o

chunker.o: ${LIB_DIR}/chunker.c
	@${CC} ${CC_FLAGS} ${LIB_DIR}/chunker.c -o ${BUILD_DIR}/chunker.o

fex.o: ${LIB_DIR}/fex.cpp
	@${CC} ${CC_FLAGS} ${LIB_DIR}/fex.cpp -o ${BUILD_DIR}/fex.o

clean:
	@rm -rf ${wildcard ./*.o ${EXEC} ${BUILD_DIR} ${DATA_DIR}/features*}

