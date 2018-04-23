//
// Created by luoqianlin on 2018/4/17.
//

#ifndef FFMPEGDEMO_LOG_H
#define FFMPEGDEMO_LOG_H

#include <android/log.h>

#define  LOG_TAG   __func__

#define AV_LOG_TAG "AV_ERR"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define  AV_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, AV_LOG_TAG, __VA_ARGS__)

#define INTERVAL_MS(start,end)  (((end)-(start))/1000.0)

inline long current_time_usec() {
    struct timeval t;
    if(gettimeofday(&t, NULL)!=0){
        perror("gettimeofday error");
        return -1;
    }
    return t.tv_sec*1000000+t.tv_usec;
}




#endif //FFMPEGDEMO_LOG_H
