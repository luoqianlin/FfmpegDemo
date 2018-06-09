package gl.context;

import android.opengl.GLES20;

import gl.tools.Tools;

public class MyGLNV12Context {

    private  final String vertexShader;
    private  final String fragmentShader;

    private final int mProgram;
    private final int mPositionHandle;
    private final int mTexCoordHandle;
    private final int textureSamplerHandleY;
    private final int textureSamplerHandleUV;
    private  int[] mNV12TexId;

    public MyGLNV12Context(){
        vertexShader = Tools.readFromAssets("NV12VertexShader.glsl");
        fragmentShader = Tools.readFromAssets("NV12FragmentShader.glsl");
        mProgram = GLES20.glCreateProgram();
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, this.vertexShader);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, this.fragmentShader);
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        mTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        textureSamplerHandleY = GLES20.glGetUniformLocation(mProgram, "yTexture");
        textureSamplerHandleUV = GLES20.glGetUniformLocation(mProgram, "uvTexture");
    }

    public void useProgram(){
        GLES20.glUseProgram(mProgram);
        if(mNV12TexId ==null) {
            mNV12TexId = new int[2];
            GLES20.glGenTextures(mNV12TexId.length, mNV12TexId, 0);

//        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mNV12TexId[0]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glUniform1i(this.getTextureSamplerHandleY(), 0);

//        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mNV12TexId[1]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glUniform1i(this.getTextureSamplerHandleUV(), 1);

        }
    }

    public void deleteProgram(){
        GLES20.glDeleteTextures(mNV12TexId.length, mNV12TexId, 0);
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


    public int[] getNV12TexId() {
        return mNV12TexId;
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

    public int getTextureSamplerHandleUV() {
        return textureSamplerHandleUV;
    }

}
