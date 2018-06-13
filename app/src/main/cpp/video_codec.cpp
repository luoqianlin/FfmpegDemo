//
// Created by luoqianlin on 2018/4/20.
//

#include <android/bitmap.h>
#include "video_codec.h"
#include <map>
#include <libyuv.h>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

inline void print_av_error(int err_code) {
    char errorMsg[1024];
    av_strerror(err_code, errorMsg, 1024);
    AV_LOGE("%s", errorMsg);
}

static map<AVCodecID,const char*>mediacodec_decoder;

#define ALOG(level, TAG, ...)    ((void)__android_log_vprint(level, TAG, __VA_ARGS__))

#define SYS_LOG_TAG "vaplayer"
static void syslog_print(void *ptr, int level, const char *fmt, va_list vl)
{
    switch(level) {
        case AV_LOG_DEBUG:
            ALOG(ANDROID_LOG_VERBOSE, SYS_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_VERBOSE:
            ALOG(ANDROID_LOG_DEBUG, SYS_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_INFO:
            ALOG(ANDROID_LOG_INFO, SYS_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_WARNING:
            ALOG(ANDROID_LOG_WARN, SYS_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_ERROR:
            ALOG(ANDROID_LOG_ERROR, SYS_LOG_TAG, fmt, vl);
            break;
    }
}

void VideoCodec::av_init(){
//    char *info = (char *)malloc(400000);
//    memset(info, 0, 400000);
    avfilter_register_all();
    av_register_all();
    avformat_network_init();
    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(syslog_print);
    av_log_set_level(AV_LOG_DEBUG);

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL)
    {
        if (c_temp->decode != NULL)
        {
//            strcat(info, "[Decode]");
        }
        else
        {
//            strcat(info, "[Encode]");
        }
        switch (c_temp->type)
        {
            case AVMEDIA_TYPE_VIDEO:
//                strcat(info, "[Video]");
//                LOGE("[Video][%s] %10s [%s] codec_id:%d\n",(c_temp->decode != NULL ? "Decode":"Encode"),
//                     c_temp->name,c_temp->long_name,c_temp->id);
                if (c_temp->decode != NULL && strstr(c_temp->name, "_mediacodec") != NULL) {
                    mediacodec_decoder.insert(pair<AVCodecID, const char *>(c_temp->id, c_temp->name));
                }
                break;

            case AVMEDIA_TYPE_AUDIO:
//                strcat(info, "[Audeo]");
//                LOGE("[Audeo][%s] %10s [%s] codec_id:%d\n",(c_temp->decode != NULL ? "Decode":"Encode"),
//                     c_temp->name,c_temp->long_name,c_temp->id);
                break;
            default:
//                strcat(info, "[Other]");
                break;
        }
//        sprintf(info, "%s %10s %s\n", info, c_temp->name,c_temp->long_name);

        c_temp = c_temp->next;
    }
    LOGE("=========start mediacodec============");
    map<AVCodecID, const char *>::iterator iter;
    for (iter = mediacodec_decoder.begin(); iter != mediacodec_decoder.end(); iter++) {
        LOGE("%d->%s\n", iter->first, iter->second);
    }
    LOGE("=========end mediacodec============");
//    LOGE("%s",info);
//    free(info);
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

    map<AVCodecID, const char *>::iterator iter = mediacodec_decoder.find(avctx->codec_id);
    if (iter != mediacodec_decoder.end()) {
        LOGE("%s %s 格式,使能硬解码",input_file.c_str(),iter->second);
        codec = avcodec_find_decoder_by_name(iter->second);
    }
//    codec = NULL;
    if (codec == NULL) {
        LOGE("%s 软解",input_file.c_str());
        codec = avcodec_find_decoder(avctx->codec_id);
    }
    if (codec == NULL) {
        LOGD("Codec not found.");
        return -1;
    }
    if ((ret = avcodec_open2(avctx, codec, NULL)) < 0) {
        LOGD("Could not open codec.");
        print_av_error(ret);
        return -1;
    }
    LOGI("id:%d,name:%s,long_name:%s",codec->id,codec->name,codec->long_name);
    this->input_file = input_file;
    this->ic = ic;
    this->avctx = avctx;
    this->stream_index = stream_index;
    this->video_height=video_height;
    this->video_width=video_width;
    LOGI("pix_fmt:%d,%d,color_trc:%d,chroma_sample_location:%d",avctx->pix_fmt,avctx->color_primaries,
         avctx->color_trc,avctx->chroma_sample_location);
    const char *primaries_name = av_color_primaries_name(avctx->color_primaries);
    LOGI("primaries_name:%s,color range:%d,color space:%d",primaries_name,avctx->color_range,avctx->colorspace);
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


#pragma clang diagnostic pop




int VideoCodec::decode_next_frame(AVFrame * &yuvFrame) {

    int y_size =this->avctx->width * this->avctx->height;
//    AVPacket *pkt = (AVPacket *) malloc(sizeof(AVPacket));
//    av_new_packet(pkt, y_size);
    AVPacket *pkt = av_packet_alloc();
    long decode_start = current_time_usec();
    yuvFrame=av_frame_alloc();
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
            const char *pix_fmt_name = av_get_pix_fmt_name(static_cast<AVPixelFormat>(yuvFrame->format));
#ifdef  AV_LOG_XXX
            LOGI("fix_fmt:%s(%d),width:%d,height:%d,linesize[0]:%d,linesize[1]:%d,linesize[2]:%d,data[0]:%p,data[1]:%p,data[1]:%p",
                 pix_fmt_name,this->avctx->pix_fmt,yuvFrame->width,yuvFrame->height,yuvFrame->linesize[0],yuvFrame->linesize[1],yuvFrame->linesize[2],
                 yuvFrame->data[0],yuvFrame->data[1],yuvFrame->data[2]
            );
            const char *primaries_name = av_color_primaries_name(yuvFrame->color_primaries);
            LOGI("av_color_primaries_name:%s,color range:%d,colorspace:%d",
                 primaries_name,yuvFrame->color_range,yuvFrame->colorspace);
#endif
            const AVCodec *codec = this->avctx->codec;
            int convert=0;
            if(codec!=NULL) {
                if (strstr(codec->name, "mpeg4") != NULL) {
                    convert = 1;
                }
//                LOGE("codec name:%s,long name:%s", codec->name, codec->long_name);
            }else{
                LOGE("codec NULL");
            }
           
//            if (this->avctx->pix_fmt != AV_PIX_FMT_YUV420P) {
            if (this->avctx->pix_fmt != AV_PIX_FMT_YUV420P
                || this->avctx->color_range != AVCOL_RANGE_UNSPECIFIED
                || this->avctx->colorspace != AVCOL_SPC_UNSPECIFIED || convert) {
                LOGW("编码不为RGBA进行转码");
                long sws_start = current_time_usec();
                convert_format(yuvFrame, AV_PIX_FMT_RGBA);
                long sws_end = current_time_usec();
                LOGI("sws cost:%.2f", (sws_end - sws_start) / 1000.0f);
            } else {
//                long sws_start = current_time_usec();
//                convert_format(yuvFrame, AV_PIX_FMT_YUV420P);
//                long sws_end = current_time_usec();
//                LOGI("sws cost:%.2f", (sws_end - sws_start) / 1000.0f);
            }
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    long decode_end = current_time_usec();
    LOGI("Native Decode Cost:%.2f",(decode_end-decode_start)/1000.0f);
    return 0;
}

void VideoCodec::convert_format(AVFrame *&yuvFrame,AVPixelFormat dst_pix_fmt) const {
    if(yuvFrame->format==AV_PIX_FMT_NV12){
        LOGD("NV12 NEON");
//            LOGE("width:%d,height:%d", yuvFrame->width, yuvFrame->height);
//            for (int i = 0; i < 3; i++) {
//                LOGE("linesize[%d]:%d,data[%d]:%p", i, yuvFrame->linesize[i], i, yuvFrame->data[i]);
//            }
        long nv2i420_start = current_time_usec();
        uint8_t *yuv_buffer = static_cast<uint8_t *>(malloc(
                (sizeof(uint8_t) * yuvFrame->height * yuvFrame->width * 3) >> 1));
        uint8_t *dst_y = yuv_buffer;

        uint8_t *dst_u = yuv_buffer + (yuvFrame->height * yuvFrame->width);

        uint8_t *dst_v = yuv_buffer + (yuvFrame->height * yuvFrame->width * 5 >> 2);

//            int dst_stride_y = yuvFrame->height * yuvFrame->width;
//            uint8_t *dst_y = static_cast<uint8_t *>(malloc(sizeof(uint8) * dst_stride_y));
//
//            int dst_stride_u = yuvFrame->height * yuvFrame->width >> 2;
//            uint8_t *dst_u = static_cast<uint8_t *>(malloc(sizeof(uint8) * dst_stride_u));
//
//            int dst_stride_v = yuvFrame->height * yuvFrame->width >> 2;
//            uint8_t *dst_v = static_cast<uint8_t *>(malloc(sizeof(uint8) * dst_stride_v));


        int toI420 = libyuv::NV12ToI420(yuvFrame->data[0], yuvFrame->width,
                                        yuvFrame->data[1], yuvFrame->width,
                                        dst_y, yuvFrame->width,
                                        dst_u, yuvFrame->width >> 1,
                                        dst_v, yuvFrame->width >> 1,
                                        yuvFrame->width, yuvFrame->height
        );
//            av_frame_unref(yuvFrame);
        yuvFrame->linesize[0] = yuvFrame->width;
        yuvFrame->data[0] = dst_y;

        yuvFrame->linesize[1] = yuvFrame->width >> 1;
        yuvFrame->data[1] = dst_u;

        yuvFrame->linesize[2] = yuvFrame->width >> 1;
        yuvFrame->data[2] = dst_v;
        yuvFrame->format = AV_PIX_FMT_YUV420P;
        if (yuvFrame->extended_data == yuvFrame->data) {
            LOGE("extend_data equal");
        }
        yuvFrame->extended_data = reinterpret_cast<uint8_t **>(yuv_buffer);
        long nv2i420_end = current_time_usec();
        LOGE("nv2i420 cost %.2f", (nv2i420_end - nv2i420_start) / 1000.0);
        if (toI420 == 0) {
            LOGE("NV12ToI420 Success");
        } else {
            LOGE("NV12ToI420 fail");
        }
    }else {
        struct SwsContext *sws_ctx = sws_getContext(avctx->width,
                                                    avctx->height,
                                                    avctx->pix_fmt,
                                                    avctx->width,
                                                    avctx->height,
                                                    dst_pix_fmt,
                                                    SWS_FAST_BILINEAR,
                                                    NULL,
                                                    NULL,
                                                    NULL);
        AVFrame *pFrameyuv = av_frame_alloc();
        pFrameyuv->width = avctx->width;
        pFrameyuv->height = avctx->height;
        pFrameyuv->format = dst_pix_fmt;
        pFrameyuv->key_frame = yuvFrame->key_frame;
//                pFrameyuv->nb_samples=yuvFrame->nb_samples;
//                pFrameyuv->channel_layout=yuvFrame->channel_layout;
//            LOGE("data[0]:%d",pFrameyuv->linesize[0]);
//                int get_buffer_ret = av_frame_get_buffer(pFrameyuv, 1);
//                if (get_buffer_ret != 0) {
//                    LOGE("av_frame_get_buffer fail");
//                }
        int numBytes = av_image_get_buffer_size(dst_pix_fmt, avctx->width,
                                                avctx->height, 1);
        uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(pFrameyuv->data, pFrameyuv->linesize, buffer, dst_pix_fmt,
                             avctx->width, avctx->height, 1);
        sws_scale(sws_ctx, (uint8_t const *const *) yuvFrame->data,
                  yuvFrame->linesize, 0, avctx->height,
                  pFrameyuv->data, pFrameyuv->linesize);
        pFrameyuv->extended_data = reinterpret_cast<uint8_t **>(buffer);
        av_frame_free(&yuvFrame);
        yuvFrame = pFrameyuv;
        sws_freeContext(sws_ctx);
    }
}

