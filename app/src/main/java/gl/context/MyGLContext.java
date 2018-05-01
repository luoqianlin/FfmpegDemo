package gl.context;

import android.opengl.GLES20;

public class MyGLContext {

    private static final String VERTEX_SHADER =
            "attribute vec4 vPosition;\n"
                    + "void main() {\n"
                    + " gl_Position = vPosition;\n"
                    + "}";
    private static final String FRAGMENT_SHADER =
            "precision mediump float;\n"
                    + "uniform vec4 vColor;\n"
                    + "void main() {\n"
                    + " gl_FragColor = vColor;\n"
                    + "}";

    private int mProgram;
    private int mPositionHandle;

    private int mColorHandle;

    public MyGLContext(){
        mProgram = GLES20.glCreateProgram();
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, VERTEX_SHADER);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        // get handle to fragment shader's vColor member
        mColorHandle = GLES20.glGetUniformLocation(mProgram, "vColor");
    }


    public void useProgram(){
        GLES20.glUseProgram(mProgram);
    }
    public void deleteProgram(){
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



    public int getPositionHandle() {
        return mPositionHandle;
    }



    public int getColorHandle() {
        return mColorHandle;
    }


}
