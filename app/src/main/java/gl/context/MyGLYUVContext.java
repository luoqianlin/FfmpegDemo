package gl.context;

import android.opengl.GLES20;

import gl.tools.Tools;

public class MyGLYUVContext {

    private  final String vertexShader;
    private  final String fragmentShader;

    private final int mProgram;
    private final int mPositionHandle;
    private final int mTexCoordHandle;
    private final int textureSamplerHandleY;
    private final int textureSamplerHandleU;
    private final int textureSamplerHandleV;
    private  int[] yuvTexId;

    /*
* GLuint aPositionHandle = (GLuint) glGetAttribLocation(programId, "aPosition");
    GLuint aTextureCoordHandle = (GLuint) glGetAttribLocation(programId, "aTexCoord");

    GLuint textureSamplerHandleY = (GLuint) glGetUniformLocation(programId, "yTexture");
    GLuint textureSamplerHandleU = (GLuint) glGetUniformLocation(programId, "uTexture");
    GLuint textureSamplerHandleV = (GLuint) glGetUniformLocation(programId, "vTexture");
* */
    public MyGLYUVContext(){
        vertexShader = Tools.readFromAssets("YuvVertexShader.glsl");
        fragmentShader = Tools.readFromAssets("YuvFragmentShader.glsl");
        mProgram = GLES20.glCreateProgram();
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, this.vertexShader);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, this.fragmentShader);
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        mTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        textureSamplerHandleY = GLES20.glGetUniformLocation(mProgram, "yTexture");
        textureSamplerHandleU = GLES20.glGetUniformLocation(mProgram, "uTexture");
        textureSamplerHandleV = GLES20.glGetUniformLocation(mProgram, "vTexture");


    }

    public void useProgram(){
        GLES20.glUseProgram(mProgram);
        if(yuvTexId==null) {
            yuvTexId = new int[3];
            GLES20.glGenTextures(3, yuvTexId, 0);

//        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[0]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_S,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glUniform1i(this.getTextureSamplerHandleY(), 0);

//        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[1]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_S,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glUniform1i(this.getTextureSamplerHandleU(), 1);

//        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[2]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_S,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glUniform1i(this.getTextureSamplerHandleV(), 2);
        }
    }

    public void deleteProgram(){
        GLES20.glDeleteTextures(3, yuvTexId, 0);
        GLES20.glDeleteProgram(mProgram);
    }

    static int loadShader(int type, String shaderCode) {
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);
        return shader;
    }

    public int getProgram() {
        return mProgram;
    }


    public int[] getYuvTexId() {
        return yuvTexId;
    }

    public int getPositionHandle() {
        return mPositionHandle;
    }



    public int getTexCoordHandle() {
        return mTexCoordHandle;
    }

    public int getTextureSamplerHandleY() {
        return textureSamplerHandleY;
    }

    public int getTextureSamplerHandleU() {
        return textureSamplerHandleU;
    }

    public int getTextureSamplerHandleV() {
        return textureSamplerHandleV;
    }
}
