#include <jni.h>
#include <string>


#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "log.h"



#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <unistd.h>


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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
JNIEXPORT jint JNICALL
Java_cn_test_ffmpegdemo_PlayActivity_play(JNIEnv *env, jobject instance, jobject surface) {

    LOGD("play");

    // sd卡中的视频文件地址,可自行修改或者通过jni传入
    //char *file_name = "/storage/emulated/0/ws2.mp4";
    const char *file_name = "/sdcard/VisualArts/materials/17bb7cb4ada9f141f70d9abeb39a8d5f.rmvb";

    av_register_all();
//    avcodec_register_all();
    avformat_network_init();

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    int ret;
    // Open video file
    if ((ret=avformat_open_input(&pFormatCtx, file_name, NULL, NULL)) != 0) {
        char errorMsg[1024];
        av_strerror(ret, errorMsg, 1024);
        LOGE("Couldn't open file:%s: %d(%s)\n", file_name,ret,errorMsg);
//        print_error(file_name,ret);
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    // Find the first video stream
    int videoStream = -1, i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStream < 0) {
            videoStream = i;
        }
    }
    if (videoStream == -1) {
        LOGE("Didn't find a video stream.");
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGD("Codec not found.");
        return -1; // Codec not found
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Could not open codec.");
        return -1; // Could not open codec
    }

    // 获取native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

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
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int got_picture;
    AVPacket * packet=av_packet_alloc();;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet->stream_index == videoStream) {

            // Decode video frame
            ret=avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret < 0){
                LOGE("Decode Error.\n");
                return -1;
            }
            // 并不是decode一次就可解码出一帧
            if (got_picture) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
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

                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }
        av_packet_unref(packet);
    }


    //FIX: Flush Frames remained in Codec
    while (1) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        if (!got_picture)
            break;
        // lock native window buffer
        ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

        // 格式转换
        sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height,
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

        ANativeWindow_unlockAndPost(nativeWindow);

    }

    LOGI("视频播放完毕");
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_free(pFrameRGBA);
    av_packet_free(&packet);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    ANativeWindow_release(nativeWindow);
    return 0;


}
#pragma clang diagnostic pop


#ifdef __cplusplus
}
#endif