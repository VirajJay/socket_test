CC_CPP = gcc
CC_NVCC = nvcc

CPPFLAGS = -std=c++17 -c
CPP_FILES = main.cpp

CU_FILES = cuda_test.cu
CUFLAGS = -c

all: cuda_app.o

cuda_app.o: cuda_kernel.o main.o
	$(CC_NVCC) $^ -o cuda_app.o $(shell pkg-config --cflags --libs libavformat libavcodec libswscale libavutil)

cuda_kernel.o: cuda_test.cu
	$(CC_NVCC) $(CUFLAGS) $(CU_FILES) -o cuda_kernel.o

main.o: main.c
	$(CC_CPP) $(CPPFLAGS) $(CPP_FILES) -o main.o

clean:
	rm -f *.o
