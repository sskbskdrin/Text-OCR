//
// Created by sskbskdrin on 2019/March/24.
//

#ifndef AUDIOVIDEO_LOG_H
#define AUDIOVIDEO_LOG_H

#include <android/log.h>

#define LOGV(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define TAG "ocr-jni"

#ifndef BOOL
#define BOOL char
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif //AUDIOVIDEO_LOG_H
