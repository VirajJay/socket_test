#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

/*
sudo apt install libavformat-dev libavcodec-dev libswscale-dev libavutil-dev
sudo apt install vlc
sudo apt install ffmpeg
gcc main.c $(pkg-config --cflags --libs libavformat libavcodec libswscale libavutil)
*/

int main(int argc, char* argv[])
{
    int red, green, blue;
    int video_stream_index = -1;
    AVCodecParameters *codec_par = NULL;
    AVCodec *decoder;
    AVCodecContext *codec_ctx;
    const char file_name[] = "videos/videoplayback.mp4";

    int retVal;

    // Open video file
    AVFormatContext *fmt_ctx = NULL;
    printf("sizeof(AVFormatContext): %ld bytes\n", sizeof(AVFormatContext));
    retVal = avformat_open_input(&fmt_ctx, file_name, NULL, NULL);
    printf("avformat_open_input retVal: %d\n", retVal);

    // Retrieve stream information
    retVal = avformat_find_stream_info(fmt_ctx, NULL);
    printf("avformat_find_stream_info retVal: %d\n", retVal);

    // Find the first video stream
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            codec_par = fmt_ctx->streams[i]->codecpar;
            printf("Video Stream: %d\n", i);
            break;
        }
    }

    printf("bit-rate: %ld\n", codec_par->bit_rate);

    // Find decoder
    decoder = avcodec_find_decoder(codec_par->codec_id);
    printf("Wrapper_Name: %s\n", decoder->name);
    printf("Wrapper_Name: %s\n", decoder->long_name);

    codec_ctx = avcodec_alloc_context3(decoder);
    retVal = avcodec_parameters_to_context(codec_ctx, codec_par);
    printf("avcodec_parameters_to_context retVal: %d\n", retVal);

    retVal = avcodec_open2(codec_ctx, decoder, NULL);
    printf("avcodec_open2 retVal: %d\n", retVal);

    // Prepare to convert frames to RGB
    struct SwsContext *sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    AVFrame *frame     = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    int num_bytes      = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t *buffer    = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    AVPacket *packet = av_packet_alloc();

    printf("codec_ctx->width: %d\ncodec_ctx->height: %d\n", codec_ctx->width, codec_ctx->height);
    uint8_t raw_rgb_frame[codec_ctx->width * codec_ctx->height * 3];

    // 1. Convert your desired time (in seconds) to timestamp units
    int64_t seek_target = 10 * AV_TIME_BASE;

    // 2. Seek (use AV_TIME_BASE for AVFormatContext, or stream's time_base if stream-specific)
    av_seek_frame(fmt_ctx, -1, seek_target, AVSEEK_FLAG_BACKWARD);

    // 3. Flush decoders (critical to avoid leftover frames)
    avcodec_flush_buffers(codec_ctx);

    // Read frames
    while (av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            retVal = avcodec_send_packet(codec_ctx, packet);
            printf("avcodec_send_packet retVal: %d\n", retVal);
            if (0 == avcodec_receive_frame(codec_ctx, frame)){
                sws_scale(sws_ctx, (const uint8_t * const*)frame->data, frame->linesize, 0,
                codec_ctx->height, rgb_frame->data, rgb_frame->linesize);
                // Now rgb_frame contains raw RGB24 data
                // You can write it to a file or process it
                printf("Decoded frame %ld\n", codec_ctx->frame_num);

                /*
                int y_index = y * y_stride + x;
                int uv_index = (y / 2) * uv_stride + (x / 2);

                uint8_t Y = frame->data[0][y_index];
                uint8_t U = frame->data[1][uv_index];
                uint8_t V = frame->data[2][uv_index];

                R = Y + 1.402    * (V - 128)
                G = Y - 0.344136 * (U - 128) - 0.714136 * (V - 128)
                B = Y + 1.772    * (U - 128)
                */

                FILE *f = fopen("frame.ppm", "wb");
                fprintf(f, "P6\n%d %d\n255\n", codec_ctx->width, codec_ctx->height);

                for(int i=0;i<codec_ctx->width * codec_ctx->height;i++)
                {
                    uint8_t Y = frame->data[0][((int)i/codec_ctx->width) * frame->linesize[0] + (i%codec_ctx->width)];
                    uint8_t U = frame->data[1][(((int)i/codec_ctx->width)/2) * frame->linesize[1] + ((i%codec_ctx->width)/2)];
                    uint8_t V = frame->data[2][(((int)i/codec_ctx->width)/2) * frame->linesize[2] + ((i%codec_ctx->width)/2)];
                    
                    red   = Y + 1.402    * (V - 128);
                    green = Y - 0.344136 * (U - 128) - 0.714136 * (V - 128);
                    blue  = Y + 1.772    * (U - 128);
                    red = red < 0 ? 0 : (red > 255 ? 255 : red);
                    green = green < 0 ? 0 : (green > 255 ? 255 : green);
                    blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);
                    raw_rgb_frame[(i*3) + 0] = red;
                    raw_rgb_frame[(i*3) + 1] = green;
                    raw_rgb_frame[(i*3) + 2] = blue;
                }

                // fwrite(rgb_frame->data[0], 1, codec_ctx->width * codec_ctx->height * 3, f);
                fwrite(raw_rgb_frame, 1, codec_ctx->width * codec_ctx->height * 3, f);
                fclose(f);

                break;
            }
        }
    }
    av_packet_unref(packet);

    // Cleanup
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_free(buffer);
    av_packet_free(&packet);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}
