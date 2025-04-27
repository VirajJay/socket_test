extern "C"{
    #include <zlib.h>    
}
#include "data_packer.hpp"
#include <cstring>
#include <iostream>

int data_size(char* input, int max_len, uint8_t magic_num) {
    int retVal = 0;

    for(int i=0;i<max_len-5;i++){
        if((char)magic_num == input[i] &&
            (char)magic_num == input[i+1] &&
            (char)magic_num == input[i+2] &&
            (char)magic_num == input[i+3] &&
            (char)magic_num == input[i+4]){
            printf("retVal: %d\n", retVal);
            printf("next one: %x\n", input[i+5]);
            break;
        }else{
            retVal++;
        }
    }

    return retVal;
}

int gzip_compress(const char *input, int input_len, unsigned char *output, int *output_len) {
    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return -1;
    }

    stream.next_in = (Bytef *)input;
    stream.avail_in = input_len;
    stream.next_out = output;
    stream.avail_out = *output_len;

    int ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&stream);
        return -2;
    }

    *output_len = stream.total_out;
    deflateEnd(&stream);
    return 0;
}
