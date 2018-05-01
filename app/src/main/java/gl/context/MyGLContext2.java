package gl.context;

import android.opengl.GLES20;

public class MyGLContext2 {

    private static final String VERTEX_SHADER =
                    "attribute vec4 vPosition;" +
                    "attribute vec2 a_texCoord;" +
                    "varying vec2 v_texCoord;" +
                    "void main() {" +
                    " gl_Position = vPosition;" +
                    " v_texCoord = a_texCoord;" +
                    "}";
    private static final String FRAGMENT_SHADER =
            "precision mediump float;" +
                    "varying vec2 v_texCoord;" +
                    "uniform sampler2D s_texture;" +
                    "void main() {" +
                    " gl_FragColor = texture2D(s_texture, v_texCoord);" +
                    "}";

    private int mProgram;
    private int mPositionHandle;
    private  int mTexCoordHandle;
    private  int mTexSamplerHandle;
    private int[] texNames;


    public MyGLContext2(){
        mProgram = GLES20.glCreateProgram();
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, VERTEX_SHADER);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        mTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "a_texCoord");
        mTexSamplerHandle = GLES20.glGetUniformLocation(mProgram, "s_texture");
    }

    public void useProgram(){
        GLES20.glUseProgram(mProgram);
        if(texNames==null) {
            texNames = new int[1];
            GLES20.glGenTextures(1, texNames, 0);
//            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texNames[0]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_REPEAT);
            GLES20.glUniform1i(this.getTexSamplerHandle(), 0);
        }
    }

    public int[] getTexNames() {
        return texNames;
    }



    public void deleteProgram(){
        GLES20.glDeleteTextures(1, texNames, 0);
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



    public int getTexCoordHandle() {
        return mTexCoordHandle;
    }

    public int getTexSamplerHandle() {
        return mTexSamplerHandle;
    }
}
