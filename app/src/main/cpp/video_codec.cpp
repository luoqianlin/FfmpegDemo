//
// Created by luoqianlin on 2018/4/20.
//

#include <android/bitmap.h>
#include "video_codec.h"



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

inline void print_av_error(int err_code) {
    char errorMsg[1024];
    av_strerror(err_code, errorMsg, 1024);
    AV_LOGE("%s", errorMsg);
}

/*
 * Write a frame worth of video (in pFrame) into the Android bitmap
 * described by info using the raw pixel buffer.  It's a very inefficient
 * draw routine, but it's easy to read. Relies on the format of the
 * bitmap being 8bits per color component plus an 8bit alpha channel.
 */

static void fill_bitmap(AndroidBitmapInfo*  info, void *pixels, AVFrame *pFrame)
{
    uint8_t *frameLine;
    LOGI("bitmap width:%d,height:%d,stride:%d", info->width,info->height,info->stride);
    int  yy;
    for (yy = 0; yy < info->height; yy++) {
        uint8_t*  line = (uint8_t*)pixels;
        frameLine = (uint8_t *)pFrame->data[0] + (yy * pFrame->linesize[0]);

        int xx;
        for (xx = 0; xx < info->width; xx++) {
            int out_offset = xx * 4;
            int in_offset = xx * 3;

            line[out_offset] = frameLine[in_offset];
            line[out_offset+1] = frameLine[in_offset+1];
            line[out_offset+2] = frameLine[in_offset+2];
            line[out_offset+3] =0xFF;
        }
        pixels = (char*)pixels + info->stride;
    }
}

VideoCodec::VideoCodec() {
    this->avctx = NULL;
    this->ic = NULL;
    this->stream_index = -1;
    this->video_width=0;
    this->video_height=0;
}

VideoCodec::~VideoCodec() {
    this->release();
}

int VideoCodec::init(const string input_file,const int video_width,const int video_height) {
    AVFormatContext *ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    int ret;

//    av_register_all();

//  avcodec_register_all();
//  avformat_network_init();

    ic = avformat_alloc_context();
    if ((ret = avformat_open_input(&ic, input_file.c_str(), NULL, NULL)) != 0) {
        LOGE("Couldn't open file:%s,ret=%d\n", input_file.c_str(), ret);
        print_av_error(ret);
        return -1;
    }

    if ((ret = avformat_find_stream_info(ic, NULL)) < 0) {
        LOGE("Couldn't find stream information.");
        print_av_error(ret);
        return -1;
    }

    int stream_index = -1, i;
    for (i = 0; i < ic->nb_streams; i++) {
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
            && stream_index < 0) {
            stream_index = i;
            AVStream *stream=ic->streams[stream_index];
            AVRational avRational=stream->avg_frame_rate;
            LOGI("rate.num:%d,rate.den:%d",avRational.num,avRational.den);
            int frame_rate=0;
            if (avRational.den != 0) {
                frame_rate = stream->avg_frame_rate.num / stream->avg_frame_rate.den;//每秒多少帧
            }
            LOGI("fps:%d",frame_rate);
        }
    }
    if (stream_index == -1) {
        LOGE("Didn't find a video stream.");
        return -1; // Didn't find a video stream
    }

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);
    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        LOGE("AVCodecContext create fail");
        print_av_error(ret);
        return -1;
    }

    av_codec_set_pkt_timebase(avctx, ic->streams[stream_index]->time_base);


    codec = avcodec_find_decoder(avctx->codec_id);
    if (codec == NULL) {
        LOGD("Codec not found.");
        return -1;
    }

    if ((ret = avcodec_open2(avctx, codec, NULL)) < 0) {
        LOGD("Could not open codec.");
        print_av_error(ret);
        return -1;
    }

    this->input_file = input_file;
    this->ic = ic;
    this->avctx = avctx;
    this->stream_index = stream_index;
    this->video_height=video_height;
    this->video_width=video_width;
    LOGI("pix_fmt:%d",avctx->pix_fmt);
    return 0;
}


int VideoCodec::release() {
    this->stream_index = -1;
    if (this->avctx) {
        avcodec_free_context(&this->avctx);
    }
    if (this->ic) {
        avformat_close_input(&this->ic);
    }
    LOGI("video release....");
    return 0;
}

int VideoCodec::decode_next_frame(char *&data_buffer, size_t &data_buff_len) {
    LOGD("decode === >被调用");
    int ret;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return -1;
    }
    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         avctx->width, avctx->height, 1);
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                avctx->height,
                                                avctx->pix_fmt,
                                                avctx->width,
                                                avctx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BICUBIC,
                                                NULL,
                                                NULL,
                                                NULL);

    int got_picture;
    AVPacket *pkt = av_packet_alloc();
    while ((ret = av_read_frame(ic, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == stream_index) {
            // Decode video frame
            ret = avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
            if (ret < 0) {
                print_av_error(ret);
                LOGE("Decode Error.\n");
                goto av_finished;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,
                     pFrameRGBA->linesize[0]);

                data_buff_len = avctx->height * avctx->width * 4;
                data_buffer = new char[data_buff_len];
                memcpy(data_buffer, pFrameRGBA->data[0], data_buff_len);
                break;
            }

        }
        av_packet_unref(pkt);
    }

    //FIX: Flush Frames remained in Codec
    if (ret == AVERROR_EOF) {
        if (avcodec_decode_video2(avctx, pFrame, &got_picture, pkt) >= 0) {
            if (got_picture) {
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
                LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,
                     pFrameRGBA->linesize[0]);
                data_buff_len = avctx->height * avctx->width * 4;
                data_buffer = new char[data_buff_len];
                memcpy(data_buffer, pFrameRGBA->data[0], data_buff_len);
            }
        }
    }

    av_finished:
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&pFrameRGBA);
    av_frame_free(&pFrame);
    av_packet_free(&pkt);

    return ret;
}


int VideoCodec::decode_next_frame_tobitmap(JNIEnv * env,jobject bitmap) {
//    LOGD("decode === >被调用");
    AndroidBitmapInfo  info;
    void*              pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return-1;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return -1;
    }


    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, info.width, info.height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
//    uint8_t * buffer= (uint8_t *) pixels;
//    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
//                         avctx->width, avctx->height, 1);
    pFrameRGBA->height=info.height;
    pFrameRGBA->width=info.width;
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         info.width, info.height, 1);
    int target_width = 320;
    int target_height = 240;
//    LOGI("taget_width:%d,taget_height=%d,src pix_fmt:%d",info.width,info.height,avctx->pix_fmt);
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                avctx->height,
                                                avctx->pix_fmt,
                                                info.width,
                                                info.height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_FAST_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int got_picture;
    AVPacket *pkt = av_packet_alloc();
    while ((ret = av_read_frame(ic, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == stream_index) {
            clock_t t1=clock();
            // Decode video frame
            ret = avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
            if (ret < 0) {
                print_av_error(ret);
                LOGE("Decode Error.\n");
                goto av_finished;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */



                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

//              LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,pFrameRGBA->linesize[0]);
//              LOGI("dest_width:%d,dest_height:%d,dest_linesize:%d", pFrameRGBA->width,
//                   pFrameRGBA->height,pFrameRGBA->linesize[0]);

//                fill_bitmap(&info, pixels, pFrameRGBA);
                clock_t  copy_start=clock();
//                LOGI("YUV430p->RGB cost:%d",(copy_start-t1));
                memcpy(pixels,pFrameRGBA->data[0],info.height*pFrameRGBA->linesize[0]);
                clock_t  t2=clock();
                LOGI("fill bitmap cost totalTime:%d,total:%d",t2-copy_start,t2-t1);
                break;
            }

        }
        av_packet_unref(pkt);
    }

    //FIX: Flush Frames remained in Codec
    if (ret == AVERROR_EOF) {
        if (avcodec_decode_video2(avctx, pFrame, &got_picture, pkt) >= 0) {
            if (got_picture) {
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
//              LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height, pFrameRGBA->linesize[0]);
//                fill_bitmap(&info, pixels, pFrameRGBA);
                memcpy(pixels,pFrameRGBA->data[0],info.height*pFrameRGBA->linesize[0]);
            }
        }
    }
    av_finished:
    AndroidBitmap_unlockPixels(env, bitmap);
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&pFrameRGBA);
    av_frame_free(&pFrame);
    av_packet_free(&pkt);

    return ret;
}



int VideoCodec::decode_next_frame(JNIEnv *env,jobject surface) {
    LOGD("decode === >被调用");
    int ret;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return -1;
    }
    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         avctx->width, avctx->height, 1);
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                avctx->height,
                                                avctx->pix_fmt,
                                                avctx->width,
                                                avctx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_FAST_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int got_picture;
    AVPacket *pkt = av_packet_alloc();

    // 获取native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = avctx->width;
    int videoHeight = avctx->height;

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    while ((ret = av_read_frame(ic, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == stream_index) {
            // Decode video frame
            ret = avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
            if (ret < 0) {
                print_av_error(ret);
                LOGE("Decode Error.\n");
                goto av_finished;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

//                LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,
//                     pFrameRGBA->linesize[0]);

                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
                LOGI("width:%d,height:%d,linesize:%d,dstStride:%d",avctx->width,avctx->height,pFrameRGBA->linesize[0],dstStride);

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst+dstStride/2 + h * dstStride, src + h * srcStride, srcStride-dstStride/2);
                }
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride, src+dstStride/2 + h * srcStride, srcStride-dstStride/2+1);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
                break;
            }

        }
        av_packet_unref(pkt);
    }

    //FIX: Flush Frames remained in Codec
    if (ret == AVERROR_EOF) {
        if (avcodec_decode_video2(avctx, pFrame, &got_picture, pkt) >= 0) {
            if (got_picture) {
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
                LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,
                     pFrameRGBA->linesize[0]);
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
                LOGI("width:%d,height:%d,linesize:%d",avctx->width,avctx->height,pFrameRGBA->linesize[0]);

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
    }
    ANativeWindow_release(nativeWindow);
    av_finished:
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&pFrameRGBA);
    av_frame_free(&pFrame);
    av_packet_free(&pkt);

    return ret;
}



int VideoCodec::decode_play(JNIEnv *env,jobject surface) {
//    LOGD("decode === >被调用");
    int ret;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return -1;
    }


    // 获取native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
//    int videoWidth = avctx->width;
//    int videoHeight = avctx->height;
//使用上面的格式，根据Android窗口，对显示进行拉伸
    uint32_t window_width  = static_cast<uint32_t>(this->video_width);
    uint32_t window_height = static_cast<uint32_t>(this->video_height);
    // 设置native window的buffer大小,可自动拉伸
    LOGI("window_width:%d,window_height:%d",window_width,window_height);
    ANativeWindow_setBuffersGeometry(nativeWindow, window_width, window_height,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, window_width,window_height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    pFrameRGBA->width=window_width;
    pFrameRGBA->height=window_height;
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         window_width,window_height, 1);
    LOGI("pix_fmt:%d",avctx->pix_fmt);
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                avctx->height,
                                                avctx->pix_fmt,
                                                window_width,
                                                window_height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_FAST_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int got_picture;
    AVPacket *pkt = av_packet_alloc();

    while ((ret = av_read_frame(ic, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == stream_index) {
            // Decode video frame
//            clock_t t1=clock();
            ret = avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
            if (ret < 0) {
                print_av_error(ret);
                LOGE("Decode Error.\n");
                goto av_finished;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
//                clock_t  tt1=clock();
//                long start=current_time_usec();
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
//                clock_t  tt2=clock();
//                long end=current_time_usec();

//                LOGI("scale:%.2f",INTERVAL_MS(start,end));
                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
//                LOGI("width:%d,height:%d,linesize:%d",avctx->width,avctx->height,pFrameRGBA->linesize[0]);
//                clock_t  c_s=clock();
//                long start_1=current_time_usec();
                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < window_height; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }
//                memcpy(dst,src,pFrameRGBA->height*pFrameRGBA->linesize[0]);
//                clock_t t2=clock();
//                long end_1=current_time_usec();
//                LOGI("copy cost:%.2f,ANativeWindow cost TotalTime:%.2f",INTERVAL_MS(start_1,end_1),
//                     INTERVAL_MS(start,end_1));
                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }
        av_packet_unref(pkt);
    }

    //FIX: Flush Frames remained in Codec
    if (ret == AVERROR_EOF) {
        if (avcodec_decode_video2(avctx, pFrame, &got_picture, pkt) >= 0) {
            if (got_picture) {
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);
                /*
                 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[]);
                 * */
                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, avctx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
//                LOGI("width:%d,height:%d,linesize:%d",avctx->width,avctx->height,pFrameRGBA->linesize[0]);

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < window_height; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
    }
    ANativeWindow_release(nativeWindow);
    av_finished:
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&pFrameRGBA);
    av_frame_free(&pFrame);
    av_packet_free(&pkt);

    return ret;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_sansi_va_VideoCodec_avInitialize(JNIEnv *env, jclass type) {
    av_register_all();
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_sansi_va_VideoCodec_init__JLjava_lang_String_2(JNIEnv *env, jobject instance, jlong ptr,
                                                        jstring file_) {
    int ret;
    const char *file = env->GetStringUTFChars(file_, 0);
    LOGD("init file:%s", file);
    ret = (reinterpret_cast<VideoCodec *>(ptr))->init(string(file),0,0);
    env->ReleaseStringUTFChars(file_, file);
    return ret;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_sansi_va_VideoCodec_init__JLjava_lang_String_2II(JNIEnv *env, jobject instance, jlong ptr,
                                                          jstring file_, jint videoWidth,
                                                          jint videoHeight) {
    int ret;
    const char *file = env->GetStringUTFChars(file_, 0);
    LOGD("init file:%s,videoWidth:%d,videoHeight:%d", file,videoWidth,videoHeight);
    ret = (reinterpret_cast<VideoCodec *>(ptr))->init(string(file),videoWidth,videoHeight);
    env->ReleaseStringUTFChars(file_, file);
    return ret;

    env->ReleaseStringUTFChars(file_, file);
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_sansi_va_VideoCodec_create(JNIEnv *env, jobject instance) {
    return (jlong) new VideoCodec();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sansi_va_VideoCodec_free(JNIEnv *env, jobject instance, jlong ptr) {
    delete reinterpret_cast<VideoCodec *>(ptr);
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_sansi_va_VideoCodec_getNextFrame__J(JNIEnv *env, jobject instance, jlong ptr) {
    char *buffer=NULL;
    size_t len=0;
    LOGD("jni nextframe 被动调用ptr=%04x",ptr);
    int ret = (reinterpret_cast<VideoCodec *>(ptr))->decode_next_frame(buffer, len);
    if (ret == AVERROR_EOF) {
        LOGI("frame 结束");
    }
    if(buffer==NULL || len <1)return NULL;
    char c;
    for (int i = 0; i < len; i += 4) {
        c = buffer[i];
        buffer[i] = buffer[i + 3];
        buffer[i + 3] = c;
    }
    jbyteArray pArray = env->NewByteArray(len);
    env->SetByteArrayRegion(pArray, 0, len, reinterpret_cast<const jbyte *>(buffer));
    delete buffer;
    return pArray;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_sansi_va_VideoCodec_display(JNIEnv *env, jobject instance, jlong ptr, jobject surface) {

    int ret = (reinterpret_cast<VideoCodec *>(ptr))->decode_next_frame(env,surface);
    if (ret == AVERROR_EOF) {
        LOGI("frame 结束");
        return 1;
    }
    return 0;

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_sansi_va_VideoCodec_fillBitmap(JNIEnv *env, jobject instance, jlong ptr, jobject bitmap) {
    int ret = (reinterpret_cast<VideoCodec *>(ptr))->decode_next_frame_tobitmap(env,bitmap);
    if (ret == AVERROR_EOF) {
        LOGI("frame 结束");
        return 1;
    }
    return 0;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sansi_va_VideoCodec_play__JLandroid_view_Surface_2(JNIEnv *env, jobject instance,
                                                            jlong ptr, jobject surface) {
    int ret = (reinterpret_cast<VideoCodec *>(ptr))->decode_play(env,surface);
    if (ret == AVERROR_EOF) {
        LOGI("frame 结束");
        return ret;
    }
    return ret;

}

#pragma clang diagnostic pop
#define GET_STR(x) #x
const char *vertexShaderString = GET_STR(
        attribute vec4 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord=vec2(aTexCoord.x,1.0-aTexCoord.y);
            gl_Position = aPosition;
        }
);
const char *fragmentShaderString = GET_STR(
        precision mediump float;
        varying vec2 vTexCoord;
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
            rgb = mat3(1.0,       1.0,         1.0,
                       0.0,       -0.39465,  2.03211,
                       1.13983, -0.58060,  0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);

void
bindYUVTexture(GLuint yTextureId, GLuint uTextureId, GLuint vTextureId, const AVFrame *yuvFrame);

extern "C"
JNIEXPORT void JNICALL
Java_cn_test_ffmpegdemo_VideoSurfaceView_videoPlay(JNIEnv *env, jobject instance, jstring path_,
                                                   jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);

    /***
     * ffmpeg 初始化
     * **/
    av_register_all();
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    if (avformat_open_input(&fmt_ctx, path, NULL, NULL) < 0) {
        return;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        return;
    }
    AVStream *avStream = NULL;
    int video_stream_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            avStream =fmt_ctx->streams[i];
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        return;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(codec_ctx, avStream->codecpar);
    AVCodec *avCodec = avcodec_find_decoder(codec_ctx->codec_id);
    if (avcodec_open2(codec_ctx, avCodec, NULL) < 0) {
        return;
    }
    LOGI("pix_fmt:%d",codec_ctx->pix_fmt);
    int y_size = codec_ctx->width * codec_ctx->height;
    AVPacket *pkt = (AVPacket *) malloc(sizeof(AVPacket));
    av_new_packet(pkt, y_size);
    /**
       *初始化egl
       **/
    EGLConfig eglConf;
    EGLSurface eglWindow;
    EGLContext eglCtx;
    int windowWidth;
    int windowHeight;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    EGLint configSpec[] = { EGL_RED_SIZE, 8,
                            EGL_GREEN_SIZE, 8,
                            EGL_BLUE_SIZE, 8,
                            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };

    EGLDisplay eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;
    eglInitialize(eglDisp, &eglMajVers, &eglMinVers);
    eglChooseConfig(eglDisp, configSpec, &eglConf, 1, &numConfigs);

    eglWindow = eglCreateWindowSurface(eglDisp, eglConf,nativeWindow, NULL);

    eglQuerySurface(eglDisp,eglWindow,EGL_WIDTH,&windowWidth);
    eglQuerySurface(eglDisp,eglWindow,EGL_HEIGHT,&windowHeight);
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    eglCtx = eglCreateContext(eglDisp, eglConf,EGL_NO_CONTEXT, ctxAttr);

    eglMakeCurrent(eglDisp, eglWindow, eglWindow, eglCtx);

    /**
     * 设置opengl 要在egl初始化后进行
     * **/
    float *vertexData= new float[12]{
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    float *textureVertexData = new float[8]{
            1.0f, 0.0f,//右下
            0.0f, 0.0f,//左下
            1.0f, 1.0f,//右上
            0.0f, 1.0f//左上
    };

    ShaderUtils *shaderUtils = new ShaderUtils();

    GLuint programId = shaderUtils->createProgram(vertexShaderString,fragmentShaderString );
    delete shaderUtils;
    GLuint aPositionHandle = (GLuint) glGetAttribLocation(programId, "aPosition");
    GLuint aTextureCoordHandle = (GLuint) glGetAttribLocation(programId, "aTexCoord");

    GLuint textureSamplerHandleY = (GLuint) glGetUniformLocation(programId, "yTexture");
    GLuint textureSamplerHandleU = (GLuint) glGetUniformLocation(programId, "uTexture");
    GLuint textureSamplerHandleV = (GLuint) glGetUniformLocation(programId, "vTexture");

    //因为没有用矩阵所以就手动自适应
    int videoWidth = codec_ctx->width;
    int videoHeight = codec_ctx->height;

    int left,top,viewWidth,viewHeight;
    if(windowHeight > windowWidth){
        left = 0;
        viewWidth = windowWidth;
        viewHeight = (int)(videoHeight*1.0f/videoWidth*viewWidth);
        top = (windowHeight - viewHeight)/2;
    }else{
        top = 0;
        viewHeight = windowHeight;
        viewWidth = (int)(videoWidth*1.0f/videoHeight*viewHeight);
        left = (windowWidth - viewWidth)/2;
    }
    glViewport(left, top, viewWidth, viewHeight);

    glUseProgram(programId);
    glEnableVertexAttribArray(aPositionHandle);
    glVertexAttribPointer(aPositionHandle, 3, GL_FLOAT, GL_FALSE,12, vertexData);
    glEnableVertexAttribArray(aTextureCoordHandle);
    glVertexAttribPointer(aTextureCoordHandle,2,GL_FLOAT,GL_FALSE,8,textureVertexData);
    /***
     * 初始化空的yuv纹理
     * **/
    GLuint yTextureId;
    GLuint uTextureId;
    GLuint vTextureId;
    glGenTextures(1,&yTextureId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,yTextureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glUniform1i(textureSamplerHandleY,0);

    glGenTextures(1,&uTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,uTextureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUniform1i(textureSamplerHandleU,1);

    glGenTextures(1,&vTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,vTextureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUniform1i(textureSamplerHandleV,2);

    struct SwsContext *sws_ctx = sws_getContext(codec_ctx->width,
                                                codec_ctx->height,
                                                codec_ctx->pix_fmt,
                                                codec_ctx->width,
                                                codec_ctx->height,
                                                AV_PIX_FMT_YUV420P,
                                                SWS_FAST_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);
    AVFrame *pFrameyuv = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codec_ctx->width, codec_ctx->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameyuv->data, pFrameyuv->linesize, buffer, AV_PIX_FMT_YUV420P,
                         codec_ctx->width, codec_ctx->height, 1);
    pFrameyuv->width=codec_ctx->width;
    pFrameyuv->height=codec_ctx->height;
    /***
     * 开始解码
     * **/
    int ret;
    while (1) {
        if (av_read_frame(fmt_ctx, pkt) < 0) {
            //播放结束
            break;
        }
        if (pkt->stream_index == video_stream_index) {
            ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_packet_unref(pkt);
                continue;
            }
            AVFrame *yuvFrame = av_frame_alloc();
            ret = avcodec_receive_frame(codec_ctx, yuvFrame);
            if (ret < 0 && ret != AVERROR_EOF) {
                av_frame_free(&yuvFrame);
                av_packet_unref(pkt);
                continue;
            }
            if(codec_ctx->pix_fmt!=AV_PIX_FMT_YUV420P) {
                sws_scale(sws_ctx, (uint8_t const *const *) yuvFrame->data,
                          yuvFrame->linesize, 0, codec_ctx->height,
                          pFrameyuv->data, pFrameyuv->linesize);
                bindYUVTexture(yTextureId, uTextureId, vTextureId, pFrameyuv);
            }else{
                bindYUVTexture(yTextureId, uTextureId, vTextureId, yuvFrame);
            }
            /***
              * 解码后的数据更新到yuv纹理中
            * **/



            /*
             *
             *
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[0], frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[0]);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[1], frame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[1]);
            glBindTexture(GL_TEXTURE_2D, texture[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[2], frame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[2]);
             */
            /***
                       * 纹理更新完成后开始绘制
                       ***/
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            eglSwapBuffers(eglDisp, eglWindow);

            av_frame_free(&yuvFrame);
        }
        av_packet_unref(pkt);
    }
    /***
     * 释放资源
     * **/
    delete[] vertexData;
    delete[] textureVertexData;

    eglMakeCurrent(eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisp, eglCtx);
    eglDestroySurface(eglDisp, eglWindow);
    eglTerminate(eglDisp);
    eglDisp = EGL_NO_DISPLAY;
    eglWindow = EGL_NO_SURFACE;
    eglCtx = EGL_NO_CONTEXT;

    avcodec_close(codec_ctx);
    avformat_close_input(&fmt_ctx);

    env->ReleaseStringUTFChars(path_, path);
}

void
bindYUVTexture(GLuint yTextureId, GLuint uTextureId, GLuint vTextureId, const AVFrame *yuvFrame) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yuvFrame->linesize[0], yuvFrame->height,0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,  yuvFrame->linesize[1], yuvFrame->height/2,0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[1]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,  yuvFrame->linesize[2], yuvFrame->height/2,0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[2]);
}

int VideoCodec::decode_next_frame(AVFrame *yuvFrame) {

    int y_size =this->avctx->width * this->avctx->height;
//    AVPacket *pkt = (AVPacket *) malloc(sizeof(AVPacket));
//    av_new_packet(pkt, y_size);
    AVPacket *pkt = av_packet_alloc();
    long decode_start = current_time_usec();
    /***
     * 开始解码
     * **/
    int ret;
    while (1) {
        if (av_read_frame(this->ic, pkt) < 0) {
            av_packet_free(&pkt);
//            free(pkt);
            return AVERROR_EOF;
        }
        if (pkt->stream_index == this->stream_index) {
            ret = avcodec_send_packet(this->avctx, pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_packet_unref(pkt);
                continue;
            }

            ret = avcodec_receive_frame(this->avctx, yuvFrame);
            if (ret < 0 && ret != AVERROR_EOF) {
                av_packet_unref(pkt);
                continue;
            }
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
//    free(pkt);
    av_packet_free(&pkt);
    long decode_end = current_time_usec();
    LOGI("Native Decode Cost:%.2f",(decode_end-decode_start)/1000.0f);
    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sansi_va_VideoCodec_nextFrame__J(JNIEnv *env, jobject instance, jlong ptr) {
    AVFrame *yuvFrame = av_frame_alloc();
    int ret = (reinterpret_cast<VideoCodec *>(ptr))->decode_next_frame(yuvFrame);
    if (ret == AVERROR_EOF) {
//        LOGI("frame 结束");
        return NULL;
    }
    jclass clazz_vaFrame = env->FindClass("com/sansi/va/VAFrame");
    jmethodID mid_vaFrame = env->GetMethodID(clazz_vaFrame, "<init>", "()V");
    jobject avFrame = env->NewObject(clazz_vaFrame, mid_vaFrame);
    jmethodID setWidth_id = env->GetMethodID(clazz_vaFrame, "setWidth", "(I)V");
    jmethodID setHeight_id = env->GetMethodID(clazz_vaFrame, "setHeight", "(I)V");
    jmethodID setLinesize_id = env->GetMethodID(clazz_vaFrame, "setLinesize", "([I)V");
    jmethodID setData_id = env->GetMethodID(clazz_vaFrame, "setData", "([Ljava/nio/Buffer;)V");

    jfieldID ptr_id = env->GetFieldID(clazz_vaFrame, "ptr", "J");

    env->CallVoidMethod(avFrame,setWidth_id,yuvFrame->width);
    env->CallVoidMethod(avFrame,setHeight_id,yuvFrame->height);

    jintArray linesize_arr = env->NewIntArray(AV_NUM_DATA_POINTERS);
    env->SetIntArrayRegion(linesize_arr,0,AV_NUM_DATA_POINTERS,yuvFrame->linesize);
    env->CallVoidMethod(avFrame,setLinesize_id,linesize_arr);
    jobjectArray byteBufferArray = env->NewObjectArray(AV_NUM_DATA_POINTERS, env->FindClass("java/nio/Buffer"), NULL);
    for(int i=0;i<AV_NUM_DATA_POINTERS;i++){
        if (yuvFrame->data[i] == NULL) {
//            LOGI("i=%d is NULL", i);
            break;
        }
        jobject byteBuffer = env->NewDirectByteBuffer(yuvFrame->data[i], yuvFrame->linesize[i]);
        env->SetObjectArrayElement(byteBufferArray,i,byteBuffer);
    }
    env->CallVoidMethod(avFrame,setData_id,byteBufferArray);
    env->SetLongField(avFrame,ptr_id,(jlong)yuvFrame);
//    LOGI("ptr:%p",yuvFrame);
//    av_frame_free(&yuvFrame);
    return avFrame;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("ffmpeg JNI_OnLoad");
    av_jni_set_java_vm(vm, reserved);
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sansi_va_VAFrame_destory__J(JNIEnv *env, jobject instance, jlong ptr) {
    AVFrame *yuvFrame= reinterpret_cast<AVFrame *>(ptr);
    av_frame_free(&yuvFrame);
}