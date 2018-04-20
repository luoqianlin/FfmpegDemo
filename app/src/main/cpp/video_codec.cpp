//
// Created by luoqianlin on 2018/4/20.
//

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
            line[out_offset+3] = 0;
        }
        pixels = (char*)pixels + info->stride;
    }
}

VideoCodec::VideoCodec() {
    this->avctx = NULL;
    this->ic = NULL;
    this->stream_index = -1;
}

VideoCodec::~VideoCodec() {
    this->release();
}

int VideoCodec::init(const string input_file) {
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
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, avctx->width, avctx->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGB24,
                         avctx->width, avctx->height, 1);
    int target_width = 320;
    int target_height = 240;
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                avctx->height,
                                                avctx->pix_fmt,
                                                avctx->width,
                                                avctx->height,
                                                AV_PIX_FMT_RGB24,
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

//              LOGI("width:%d,height:%d,linesize:%d", avctx->width, avctx->height,pFrameRGBA->linesize[0]);
                fill_bitmap(&info, pixels, pFrameRGBA);
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
                fill_bitmap(&info, pixels, pFrameRGBA);
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
                                                SWS_BICUBIC,
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
    ret = (reinterpret_cast<VideoCodec *>(ptr))->init(string(file));
    env->ReleaseStringUTFChars(file_, file);
    return ret;
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