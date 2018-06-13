//
// Created by luoqianlin on 2018/4/20.
//

#ifndef FFMPEGDEMO_VIDEO_CODEC_H
#define FFMPEGDEMO_VIDEO_CODEC_H

#include <string>
#include <iostream>
#include <unistd.h>

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>
#include "log.h"

//#include <GLES/egl.h>
//#include <GLES/gl.h>
//#include <GLES/glext.h>
//#include <GLES/glplatform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>


//#include <GLES3/gl3.h>
//#include <GLES3/gl31.h>
//#include <GLES3/gl32.h>
//#include <GLES3/gl3ext.h>
//#include <GLES3/gl3platform.h>

#include "shaderUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

#include <libavcodec/jni.h>

#ifdef __cplusplus
}
#endif

using namespace std;

class VideoCodec{

private :
    string input_file;
    AVFormatContext *ic;
    AVCodecContext *avctx;
    int stream_index;
    int video_width;
    int video_height;
public:
    VideoCodec();
    ~VideoCodec();
    int init(const string inpput_file,const int video_width,const int video_height);
    int release();
    int decode_next_frame(char* &buffer,size_t &buff_len);
    int decode_next_frame(JNIEnv *env,jobject surface);
    int decode_next_frame_tobitmap(JNIEnv * env,jobject bitmap);
    int decode_play(JNIEnv *env,jobject surface);

    int decode_next_frame(AVFrame*&av_frame);

    void convert_format(AVFrame *&yuvFrame, AVPixelFormat dst_pix_fmt) const;
    static void av_init();
    int get_video_raw_height();

    int get_video_raw_width();
};


#endif //FFMPEGDEMO_VIDEO_CODEC_H
