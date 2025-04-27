#ifndef DATA_PACKER_H_
#define DATA_PACKER_H_

#include <stdint.h>

int data_size(char* input, int max_len, uint8_t magic_num);
int gzip_compress(const char *input, int input_len, unsigned char *output, int *output_len);

#endif