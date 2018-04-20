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


#ifdef __cplusplus
}
#endif

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
