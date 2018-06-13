#include <jni.h>
#include <string>


#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "log.h"
#include "video_codec.h"



#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <unistd.h>
#include <libswresample/swresample.h>




JNIEXPORT jstring
JNICALL
Java_cn_test_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    LOGI("信息日志");
    LOGE("错误日志");
    LOGD("DEBUG日志");
    LOGW("警告日志");
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jstring JNICALL
Java_cn_test_ffmpegdemo_MainActivity_urlprotocolinfo(JNIEnv *env, jobject instance) {
    char info[40000] = {0};
    av_register_all();
    struct URLProtocol *pup = NULL;

    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **) p_temp, 0);

    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 1));
    }
    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cn_test_ffmpegdemo_MainActivity_avformatinfo(JNIEnv *env, jobject instance) {
    char info[40000] = {0};

    av_register_all();

    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    while (if_temp != NULL) {
        sprintf(info, "%sInput: %s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    while (of_temp != NULL) {
        sprintf(info, "%sOutput: %s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cn_test_ffmpegdemo_MainActivity_avcodecinfo(JNIEnv *env, jobject instance) {
    char info[40000] = {0};

    av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }

    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cn_test_ffmpegdemo_MainActivity_avfilterinfo(JNIEnv *env, jobject instance) {
    char info[40000] = {0};
    avfilter_register_all();

    AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);
    while (f_temp != NULL) {
        sprintf(info, "%s%s\n", info, f_temp->name);
        f_temp = f_temp->next;
    }
    return env->NewStringUTF(info);
}




void print_error(const char *filename, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}
bool save_pic(AVFrame *frm, AVCodecContext *pCodecCtx, AVCodecID cid, const char* filename);
#include <string>
int convert_first_frame_to_png(std::string const & inputVideoFileName, std::string const & outputPngName);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
JNIEXPORT jint JNICALL
Java_cn_test_ffmpegdemo_PlayActivity_play(JNIEnv *env, jobject instance, jobject surface) {
    LOGD("play");
    int ret;
    AVFormatContext *ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    // sd卡中的视频文件地址,可自行修改或者通过jni传入
    //char *file_name = "/storage/emulated/0/ws2.mp4";
    const char *file_name = "/sdcard/Wildlife.wmv";
//    std::string file_out_name="/sdcard/xx_first_frame.png";
//    convert_first_frame_to_png(std::string(file_name),file_out_name);

    av_register_all();
//    avcodec_register_all();
//    avformat_network_init();

    ic= avformat_alloc_context();
    // Open video file
    if ((ret = avformat_open_input(&ic, file_name, NULL, NULL)) != 0) {
        char errorMsg[1024];
        av_strerror(ret, errorMsg, 1024);
        LOGE("Couldn't open file:%s: %d(%s)\n", file_name, ret, errorMsg);
//        print_error(file_name,ret);
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(ic, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    // Find the first video stream
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
        return -1;
    }

    av_codec_set_pkt_timebase(avctx, ic->streams[stream_index]->time_base);
    // Get a pointer to the codec context for the video stream
//    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    codec = avcodec_find_decoder(avctx->codec_id);
    if (codec == NULL) {
        LOGD("Codec not found.");
        return -1; // Codec not found
    }

    if (avcodec_open2(avctx, codec, NULL) < 0) {
        LOGD("Could not open codec.");
        return -1; // Could not open codec
    }

    // 获取native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = avctx->width;
    int videoHeight = avctx->height;

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

//    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
//        LOGD("Could not open codec.");
//        return -1; // Could not open codec
//    }

    // Allocate video frame
    AVFrame *pFrame = av_frame_alloc();


    // 用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return -1;
    }

    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height,1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         avctx->width, avctx->height, 1);
    int t_t=0;
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
    AVPacket * pkt=av_packet_alloc();
    while (av_read_frame(ic, pkt) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == stream_index) {

            // Decode video frame
            ret=avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
            if(ret < 0){
                LOGE("Decode Error.\n");
                return -1;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {
                // lock native window buffer
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

                // Determine required buffer size and allocate buffer
                // buffer中数据就是用于渲染的,且格式为RGBA
//                int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height,
//                                                        1);
//                uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
//                av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
//                                     avctx->width, avctx->height, 1);
//                FILE * outPng = fopen("/sdcard/llll.png", "wb");
//                fwrite(buffer, numBytes, 1, outPng);
//                fclose(outPng);
//                av_free(buffer);
//                LOGE("format:%d,%d",pFrameRGBA->format,pFrame->format);
//                if (t_t % 30 == 0) {
//                    struct timeval tv;
//                    gettimeofday(&tv, NULL);
//                    char filename[50];
//                    sprintf(filename, "%s%ld.png", "/sdcard/xxx___", tv.tv_usec);
//                    LOGE("format:%d",pFrameRGBA->format);
//                    save_pic(pFrame,avctx, AV_CODEC_ID_PNG, filename);
//                }
                ANativeWindow_unlockAndPost(nativeWindow);
                t_t++;
            }

        }
        av_packet_unref(pkt);
    }


    //FIX: Flush Frames remained in Codec
    while (1) {
        ret = avcodec_decode_video2(avctx, pFrame, &got_picture, pkt);
        if (ret < 0)
            break;
        if (!got_picture)
            break;
        // lock native window buffer
        ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

        // 格式转换
        sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                  pFrame->linesize, 0, avctx->height,
                  pFrameRGBA->data, pFrameRGBA->linesize);

        // 获取stride
        uint8_t *dst = (uint8_t *) windowBuffer.bits;
        int dstStride = windowBuffer.stride * 4;
        uint8_t *src = (pFrameRGBA->data[0]);
        int srcStride = pFrameRGBA->linesize[0];

        // 由于window的stride和帧的stride不同,因此需要逐行复制
        int h;
        for (h = 0; h < videoHeight; h++) {
            memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
        }
//        if (t_t % 30 == 0) {
//            struct timeval tv;
//            gettimeofday(&tv, NULL);
//            char filename[50];
//            sprintf(filename, "%s%ld.png", "/sdcard/xxx___", tv.tv_usec);
//            save_pic(pFrame, AV_PIX_FMT_RGB24, AV_CODEC_ID_PNG, filename, avctx->width,
//                     avctx->height);
//        }
        ANativeWindow_unlockAndPost(nativeWindow);

    }

    LOGI("视频播放完毕");
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&pFrameRGBA);
    // Free the YUV frame
    av_frame_free(&pFrame);
    av_packet_free(&pkt);

    // Close the codecs
    avcodec_free_context(&avctx);

    // Close the video file
    avformat_close_input(&ic);
    ANativeWindow_release(nativeWindow);
    return 0;


}
#pragma clang diagnostic pop

#define CHECK_ERR(ERR) {if ((ERR)<0) return -1; }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
int convert_first_frame_to_png(std::string const & inputVideoFileName, std::string const & outputPngName)
{
    av_register_all();
//    avcodec_register_all();

    AVFormatContext * ctx = NULL;
    int err = avformat_open_input(&ctx, inputVideoFileName.c_str(), NULL, NULL);
    CHECK_ERR(err);
    err = avformat_find_stream_info(ctx, NULL);
    CHECK_ERR(err);

    AVCodec * codec = NULL;
    int strm = av_find_best_stream(ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);

    AVCodecContext * codecCtx = ctx->streams[strm]->codec;
    err = avcodec_open2(codecCtx, codec, NULL);
    CHECK_ERR(err);

    SwsContext * swCtx = sws_getContext(codecCtx->width,
                                        codecCtx->height,
                                        codecCtx->pix_fmt,
                                        codecCtx->width,
                                        codecCtx->height,
                                        AV_PIX_FMT_RGBA,
                                        SWS_FAST_BILINEAR, 0, 0, 0);

    for (;;)
    {
        AVPacket pkt;
        err = av_read_frame(ctx, &pkt);
        CHECK_ERR(err);

        if (pkt.stream_index == strm)
        {
            int got = 0;
            AVFrame * frame = av_frame_alloc();
            err = avcodec_decode_video2(codecCtx, frame, &got, &pkt);
            CHECK_ERR(err);

            if (got)
            {
                AVFrame * rgbFrame = av_frame_alloc();
                avpicture_alloc((AVPicture *)rgbFrame, AV_PIX_FMT_RGBA, codecCtx->width, codecCtx->height);
                sws_scale(swCtx, (uint8_t const *const *)frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);

                AVCodec *outCodec = avcodec_find_encoder(AV_CODEC_ID_PNG);
                AVCodecContext *outCodecCtx = avcodec_alloc_context3(codec);
                if (!outCodecCtx)
                    return -1;

                outCodecCtx->width = codecCtx->width;
                outCodecCtx->height = codecCtx->height;
                outCodecCtx->pix_fmt = AV_PIX_FMT_RGB24;
                outCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
                outCodecCtx->time_base.num = codecCtx->time_base.num;
                outCodecCtx->time_base.den = codecCtx->time_base.den;

                if (!outCodec || (err=avcodec_open2(outCodecCtx, outCodec, NULL)) < 0) {
                    char errorMsg[1024];
                    av_strerror(err, errorMsg, 1024);
                    LOGE("Couldn't open file: %d(%s)\n", err, errorMsg);
                    return -1;
                }

                AVPacket outPacket;
                av_init_packet(&outPacket);
                outPacket.size = 0;
                outPacket.data = NULL;
                int gotFrame = 0;
                int ret = avcodec_encode_video2(outCodecCtx, &outPacket, rgbFrame, &gotFrame);
                if (ret >= 0 && gotFrame)
                {
                    FILE * outPng = fopen(outputPngName.c_str(), "wb");
                    fwrite(outPacket.data, outPacket.size, 1, outPng);
                    fclose(outPng);
                    LOGE("write file success:%s",outputPngName.c_str());
                }

                avcodec_close(outCodecCtx);
                av_free(outCodecCtx);

                break;
            }
            av_frame_free(&frame);
        }
    }
    return 0;
}
#pragma clang diagnostic pop




extern "C"
JNIEXPORT void JNICALL
Java_cn_test_ffmpegdemo_MainActivity_getfirstframe(JNIEnv *env, jobject instance) {

    const char *file_name = "/sdcard/08.Java多线程.mp4";
    std::string file_out_name="/sdcard/xx_first_frame.png";
    convert_first_frame_to_png(std::string(file_name),file_out_name);


}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool save_pic(AVFrame *frm,  AVCodecContext *  pCodecCtx, AVCodecID cid, const char* filename)
{
    int outbuf_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);;
    uint8_t * outbuf =(uint8_t *) malloc(outbuf_size)   ;
    int got_pkt = 0;
    FILE* pf;
    pf = fopen(filename, "wb");
    if (pf == NULL)
        return false;
    AVPacket pkt;
    AVCodec *pCodecRGB24;
    AVCodecContext *ctx = NULL;
    pCodecRGB24 = avcodec_find_encoder(cid);
    if (!pCodecRGB24)
        return false;
    ctx = avcodec_alloc_context3(pCodecRGB24);
    ctx->bit_rate = pCodecCtx->bit_rate;
    ctx->width = pCodecCtx->width;
    ctx->height = pCodecCtx->height;
    ctx->pix_fmt = pCodecCtx->pix_fmt;
    ctx->codec_id = cid;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    LOGI("pCodecCtx->time_base.num:%d",pCodecCtx->time_base.num);
    ctx->time_base.num = 1;
    ctx->time_base.den = pCodecCtx->time_base.den;


    int ret = avcodec_open2(ctx, pCodecRGB24, NULL);
    if (ret < 0) {
        char errorMsg[1024];
        av_strerror(ret, errorMsg, 1024);
        LOGE("Couldn't open codec:%d(%s)\n",ret, errorMsg);
        goto end;
    }

//  int size = ctx->width * ctx->height * 4;
    av_init_packet(&pkt);
    static int got_packet_ptr = 0;
    pkt.size = outbuf_size;
    pkt.data = outbuf;
    got_pkt = avcodec_encode_video2(ctx, &pkt, frm, &got_packet_ptr);
    frm->pts++;
    if (got_pkt == 0)
    {
        size_t w_sized = fwrite(pkt.data, 1, pkt.size, pf);
        LOGE("写入数据:%s,%d",filename,w_sized);
    }
    else
    {
        return false;
    }
    end:
    avcodec_close(ctx);
    free(outbuf);
    fclose(pf);
    return true;
}
#pragma clang diagnostic pop



extern "C"
JNIEXPORT void JNICALL
Java_com_sansi_va_VideoCodec_avInitialize(JNIEnv *env, jclass type) {
    VideoCodec::av_init();
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
    LOGD("jni nextframe 被动调用ptr=%04lx",ptr);
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


extern "C"
JNIEXPORT jobject JNICALL
Java_com_sansi_va_VideoCodec_nextFrame__J(JNIEnv *env, jobject instance, jlong ptr) {
    AVFrame *yuvFrame = NULL;
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
    jmethodID  setFormat_id = env->GetMethodID(clazz_vaFrame,"setFormat","(I)V");

    jfieldID ptr_id = env->GetFieldID(clazz_vaFrame, "ptr", "J");

    env->CallVoidMethod(avFrame,setWidth_id,yuvFrame->width);
    env->CallVoidMethod(avFrame,setHeight_id,yuvFrame->height);
    env->CallVoidMethod(avFrame,setFormat_id,yuvFrame->format);

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
//    LOGE("%p \n",(void*)ptr);
    AVFrame *yuvFrame = reinterpret_cast<AVFrame *>(ptr);
//    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
//        if (yuvFrame->data[i]) {
//            LOGD("%p",(void*)yuvFrame->data[i]);
//            av_free(yuvFrame->data[i]);
//            yuvFrame->data[i] = NULL;
//        }
//    }
    av_frame_free(&yuvFrame);
}

#define GET_STR(x) #x
const char *vertexShaderString = GET_STR(
        attribute vec4 aPosition;
        attribute mediump vec2 aTexCoord;
        varying mediump vec2 vTexCoord;
        void main() {
            vTexCoord=vec2(aTexCoord.x,1.0-aTexCoord.y);
            gl_Position = aPosition;
        }
);
const char *fragmentShaderString = GET_STR(
        precision mediump float;
        varying highp vec2 vTexCoord;
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;

        const vec3 delyuv = vec3(-0.0 / 255.0, -128.0 / 255.0, -128.0 / 255.0);
        const vec3 matYUVRGB1 = vec3(1.0, 0.0, 1.402);
        const vec3 matYUVRGB2 = vec3(1.0, -0.344, -0.714);
        const vec3 matYUVRGB3 = vec3(1.0, 1.772, 0.0);

        void main() {
            vec3 curResult;
            highp vec3 yuv;
            yuv.x = texture2D(yTexture, vTexCoord).r;
            yuv.y = texture2D(uTexture, vTexCoord).r;
            yuv.z = texture2D(vTexture, vTexCoord).r;

            yuv += delyuv;

            curResult.x = dot(yuv,matYUVRGB1);
            curResult.y = dot(yuv,matYUVRGB2);
            curResult.z = dot(yuv,matYUVRGB3);

            gl_FragColor = vec4(curResult.rgb, 1.0);
        }
);

void bindYUVTexture(GLuint yTextureId, GLuint uTextureId, GLuint vTextureId, const AVFrame *yuvFrame);

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
//    glViewport(left, top, viewWidth, viewHeight);
    glViewport(0, 0, videoWidth,videoHeight);

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
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);


    glUniform1i(textureSamplerHandleY,0);

    glGenTextures(1,&uTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,uTextureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glUniform1i(textureSamplerHandleU,1);

    glGenTextures(1,&vTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,vTextureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

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
            if (codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P) {
                LOGE("不是YUV420P，进行转换");
                sws_scale(sws_ctx, (uint8_t const *const *) yuvFrame->data,
                          yuvFrame->linesize, 0, codec_ctx->height,
                          pFrameyuv->data, pFrameyuv->linesize);
                bindYUVTexture(yTextureId, uTextureId, vTextureId, pFrameyuv);
            } else {
//                LOGE("已经是YUV420P");
                if(yuvFrame->key_frame == 1){
                    LOGI("关键帧");
                }else{
                    LOGE("非关键帧:%d",yuvFrame->key_frame);
                }
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





void bindYUVTexture(GLuint yTextureId, GLuint uTextureId, GLuint vTextureId, const AVFrame *yuvFrame) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yuvFrame->linesize[0], yuvFrame->height,0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,  yuvFrame->linesize[1], yuvFrame->height/2,0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[1]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,  yuvFrame->linesize[2], yuvFrame->height/2,0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvFrame->data[2]);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_sansi_va_VideoCodec_getVideoRawHeight__J(JNIEnv *env, jobject instance, jlong ptr) {
    return (reinterpret_cast<VideoCodec *>(ptr))->get_video_raw_height();

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sansi_va_VideoCodec_getVideoRawWidth__J(JNIEnv *env, jobject instance, jlong ptr) {
    return (reinterpret_cast<VideoCodec *>(ptr))->get_video_raw_width();
}



#ifdef __cplusplus
}
#endif