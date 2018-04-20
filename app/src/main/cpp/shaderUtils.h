//
// Created by luo on 18-4-20.
//

#ifndef FFMPEGDEMO_SHADERUTILS_H_H
#define FFMPEGDEMO_SHADERUTILS_H_H

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
#include <stdio.h>
#include <malloc.h>



class ShaderUtils {
public:
    GLuint createProgram(const char *vertexSource, const char *fragmentSource);

    GLuint loadShader(GLenum shaderType, const char *source);
};
#endif //FFMPEGDEMO_SHADERUTILS_H_H
