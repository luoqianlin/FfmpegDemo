//
// Created by luoqianlin on 2018/4/20.
//

#ifndef FFMPEGDEMO_VIDEO_CODEC_H
#define FFMPEGDEMO_VIDEO_CODEC_H

#include <string>
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

#ifdef __cplusplus
}
using namespace std;

class VideoCodec{

private :
    string input_file;
    AVFormatContext *ic;
    AVCodecContext *avctx;
    int stream_index;
public:
    VideoCodec();
    ~VideoCodec();
    int init(const string inpput_file);
    int release();
    int decode_next_frame(char* &buffer,size_t &buff_len);
    int decode_next_frame(JNIEnv *env,jobject surface);
    int decode_next_frame_tobitmap(JNIEnv * env,jobject bitmap);
    int decode_play(JNIEnv *env,jobject surface);

};

#endif
#endif //FFMPEGDEMO_VIDEO_CODEC_H
