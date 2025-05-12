#include <cuda_runtime.h>
#include <iostream>
#include <cstdint>
extern "C"{
#include <libavformat/avformat.h>
}

__global__ void frame_proc_gpu(uint8_t* y_frame, int y_frame_size,
                                uint8_t* u_frame, int u_frame_size,
                                uint8_t* v_frame, int v_frame_size,
                                uint8_t* data, int width, int height) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int red, green, blue;
    if(idx < width*height){
        uint8_t Y = y_frame[(((int)idx/width) * y_frame_size) + (idx%width)];
        uint8_t U = u_frame[((((int)idx/width)/2) * u_frame_size) + ((idx%width)/2)];
        uint8_t V = v_frame[((((int)idx/width)/2) * v_frame_size) + ((idx%width)/2)];

        red   = Y + 1.402    * (V - 128);
        green = Y - 0.344136 * (U - 128) - 0.714136 * (V - 128);
        blue  = Y + 1.772    * (U - 128);
        red = red < 0 ? 0 : (red > 255 ? 255 : red);
        green = green < 0 ? 0 : (green > 255 ? 255 : green);
        blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);
        data[(idx*3) + 0] = red;
        data[(idx*3) + 1] = green;
        data[(idx*3) + 2] = blue;
    }
}

int frame_proc(AVFrame *frame, uint8_t *raw_rgb_frame, int raw_rgb_frame_len, int frame_width, int frame_height){
    int retVal = 0;
    int size = frame_width * frame_height * 3;

    int total_elements = frame_width * frame_height;
    int threads_per_block = 256;

    if(raw_rgb_frame_len == size){
        /* Y-Frame */
        uint8_t *y_frame;
        cudaMalloc(&y_frame, frame_height*frame->linesize[0]);
        cudaMemcpy(y_frame, frame->data[0], frame_height*frame->linesize[0], cudaMemcpyHostToDevice);
        printf("frame->linesize[0]: %d\n", frame->linesize[0]);
        /* U-Frame */
        uint8_t *u_frame;
        cudaMalloc(&u_frame, frame_height*frame->linesize[1]);
        cudaMemcpy(u_frame, frame->data[1], frame_height*frame->linesize[1], cudaMemcpyHostToDevice);
        printf("frame->linesize[1]: %d\n", frame->linesize[1]);
        /* V-Frame */
        uint8_t *v_frame;
        cudaMalloc(&v_frame, frame_height*frame->linesize[2]);
        cudaMemcpy(v_frame, frame->data[2], frame_height*frame->linesize[2], cudaMemcpyHostToDevice);
        printf("frame->linesize[2]: %d\n", frame->linesize[2]);

        uint8_t* d_data;
        cudaMalloc(&d_data, size);

        int num_blocks = total_elements / threads_per_block;

        // Launch kernel
        frame_proc_gpu<<<num_blocks, threads_per_block>>>(y_frame, frame->linesize[0],
                                                            u_frame, frame->linesize[1],
                                                            v_frame, frame->linesize[2],
                                                            d_data, frame_width, frame_height);
        cudaDeviceSynchronize();

        // Copy result back
        cudaMemcpy(raw_rgb_frame, d_data, size, cudaMemcpyDeviceToHost);
        cudaFree(d_data);
        cudaFree(y_frame);
        cudaFree(u_frame);
        cudaFree(v_frame);

        retVal = 0;
    }else{
        retVal = -1;
    }

    return retVal;
}
