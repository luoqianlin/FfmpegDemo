# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
              src/main/cpp/native-lib.cpp
              src/main/cpp/video_codec.cpp
              src/main/cpp/shaderUtils.cpp
             )
add_library(
            avcodec
            SHARED
            IMPORTED
            )

add_library(
            avfilter
            SHARED
            IMPORTED
             )


add_library(
            avformat
            SHARED
            IMPORTED
            )


add_library(
            avutil
            SHARED
            IMPORTED
            )

add_library(
            swresample
            SHARED
            IMPORTED
            )

add_library(
            swscale
            SHARED
            IMPORTED
            )

if(${ANDROID_ABI} STREQUAL "armeabi")
set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libavcodec.so
    )

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libavfilter.so
        )

set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libavformat.so
            )

set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libavutil.so
            )

set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libswresample.so
             )

set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi/libswscale.so
             )

endif(${ANDROID_ABI} STREQUAL "armeabi")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")

add_library( # Sets the name of the library.
             yuv

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
                src/main/cpp/libyuv/compare.cc
                src/main/cpp/libyuv/compare_common.cc
                src/main/cpp/libyuv/compare_gcc.cc
                src/main/cpp/libyuv/compare_msa.cc
                src/main/cpp/libyuv/compare_neon.cc
                src/main/cpp/libyuv/compare_neon64.cc
                src/main/cpp/libyuv/convert.cc
                src/main/cpp/libyuv/convert_argb.cc
                src/main/cpp/libyuv/convert_from.cc
                src/main/cpp/libyuv/convert_from_argb.cc
                src/main/cpp/libyuv/convert_to_argb.cc
                src/main/cpp/libyuv/convert_to_i420.cc
                src/main/cpp/libyuv/cpu_id.cc
                src/main/cpp/libyuv/planar_functions.cc
                src/main/cpp/libyuv/rotate.cc
                src/main/cpp/libyuv/rotate_any.cc
                src/main/cpp/libyuv/rotate_argb.cc
                src/main/cpp/libyuv/rotate_common.cc
                src/main/cpp/libyuv/rotate_gcc.cc
                src/main/cpp/libyuv/rotate_msa.cc
                src/main/cpp/libyuv/rotate_neon.cc
                src/main/cpp/libyuv/rotate_neon64.cc
                src/main/cpp/libyuv/row_any.cc
                src/main/cpp/libyuv/row_common.cc
                src/main/cpp/libyuv/row_gcc.cc
                src/main/cpp/libyuv/row_msa.cc
                src/main/cpp/libyuv/row_neon.cc
                src/main/cpp/libyuv/row_neon64.cc
                src/main/cpp/libyuv/scale.cc
                src/main/cpp/libyuv/scale_any.cc
                src/main/cpp/libyuv/scale_argb.cc
                src/main/cpp/libyuv/scale_common.cc
                src/main/cpp/libyuv/scale_gcc.cc
                src/main/cpp/libyuv/scale_msa.cc
                src/main/cpp/libyuv/scale_neon.cc
                src/main/cpp/libyuv/scale_neon64.cc
                src/main/cpp/libyuv/video_common.cc
                src/main/cpp/libyuv/convert_jpeg.cc
                src/main/cpp/libyuv/mjpeg_decoder.cc
                src/main/cpp/libyuv/mjpeg_validate.cc
             )

set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libavcodec.so
    )

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libavfilter.so
        )

set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libavformat.so
            )

set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libavutil.so
            )

set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libswresample.so
             )

set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libswscale.so
             )

endif(${ANDROID_ABI} STREQUAL "armeabi-v7a")

if(${ANDROID_ABI} STREQUAL "arm64-v8a")
set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libavcodec.so
    )

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libavfilter.so
        )

set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libavformat.so
            )

set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libavutil.so
            )

set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libswresample.so
             )

set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libswscale.so
             )



endif(${ANDROID_ABI} STREQUAL "arm64-v8a")

if(${ANDROID_ABI} STREQUAL "x86")
set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libavcodec.so
    )

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libavfilter.so
        )

set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libavformat.so
            )

set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libavutil.so
            )

set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libswresample.so
             )

set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86/libswscale.so
             )

endif(${ANDROID_ABI} STREQUAL "x86")

if(${ANDROID_ABI} STREQUAL "x86_64")
set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libavcodec.so
    )

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libavfilter.so
        )

set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libavformat.so
            )

set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libavutil.so
            )

set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libswresample.so
             )

set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/jniLibs/x86_64/libswscale.so
             )

endif(${ANDROID_ABI} STREQUAL "x86_64")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

include_directories(
   ${CMAKE_SOURCE_DIR}/src/main/jniLibs/include
)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
find_library( # Sets the name of the path variable.
              android-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              android )
find_library( # Sets the name of the path variable.
                EGL-lib

                # Specifies the name of the NDK library that
                # you want CMake to locate.
                EGL )

find_library( # Sets the name of the path variable.
                GLES2-lib

                # Specifies the name of the NDK library that
                # you want CMake to locate.
                GLESv2)

find_library( # Sets the name of the path variable.
                GLES3-lib

                # Specifies the name of the NDK library that
                # you want CMake to locate.
                GLESv3)



# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       yuv
                       avcodec
                       avfilter
                       avformat
                       avutil
                       swresample
                       swscale
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib}
                       ${EGL-lib}
                       ${GLES2-lib}
                       ${GLES3-lib}
                       )