package gl.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import gl.tools.GLHelper;
import gl.tools.Tools;

public class MyRenderer2 implements GLSurfaceView.Renderer {

    private static int TEXTURE_ID = 0;
    private static int TEXTURE_WIDTH = 1;
    private static int TEXTURE_HEIGHT = 2;
    private int program;
    private int attribPosition;
    private int attribTexCoord;
    private int uniformTexture;
    private int textureId;

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glEnable(GLES20.GL_TEXTURE_2D);
        // Active the texture unit 0
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        loadVertex();
        initShader();
        textureId = loadTexture("ic_launcher.png")[0];
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0,0,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
// clear screen to black
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        vertex.position(0);
// load the position
// 3(x , y , z)
// (2 + 3 )* 4 (float size) = 20
        GLES20.glVertexAttribPointer(attribPosition,
                3, GLES20.GL_FLOAT,
                false, 20, vertex);
        vertex.position(3);
// load the texture coordinate
        GLES20.glVertexAttribPointer(attribTexCoord,
                2, GLES20.GL_FLOAT,
                false, 20, vertex);
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, 6, GLES20.GL_UNSIGNED_SHORT,
                index);
    }

    private void loadVertex() {

// float size = 4
        this.vertex = ByteBuffer.allocateDirect(quadVertex.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        this.vertex.put(quadVertex).position(0);
// short size = 2
        this.index = ByteBuffer.allocateDirect(quadIndex.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer();
        this.index.put(quadIndex).position(0);
    }

    private FloatBuffer vertex;
    private ShortBuffer index;
    private float[] quadVertex = new float[]{
            1f, 0.5f, 0.0f, // Position 0
            1.0f, 0.0f, // TexCoord 0
            -1.0f, 0.5f, 0.0f, // Position 1
            0, 0, // TexCoord 1
            -1.0f, -0.5f, 0.0f, // Position 2
            0.0f, 1.0f, // TexCoord 2
            1.0f, -0.5f, 0.0f, // Position 3
            1.0f, 1.0f, // TexCoord 3
    };
    private short[] quadIndex = new short[]{
            (short) (0), // Position 0
            (short) (1), // Position 1
            (short) (2), // Position 2
            (short) (0), // Position 2
            (short) (2), // Position 3
            (short) (3), // Position 0
    };

    private void initShader() {

        String vertexSource = Tools.readFromAssets("VertexShader.glsl");
        String fragmentSource = Tools.readFromAssets("FragmentShader.glsl");
// Load the shaders and get a linked program
        program = GLHelper.loadProgram(vertexSource, fragmentSource);
// Get the attribute locations
        attribPosition = GLES20.glGetAttribLocation(program, "a_position");
        attribTexCoord = GLES20.glGetAttribLocation(program, "a_texCoord");
        uniformTexture = GLES20.glGetUniformLocation(program, "u_samplerTexture");
        GLES20.glUseProgram(program);
        GLES20.glEnableVertexAttribArray(attribPosition);
        GLES20.glEnableVertexAttribArray(attribTexCoord);
// Set the sampler to texture unit 0
        GLES20.glUniform1i(uniformTexture, 0);
    }


    static int[] loadTexture(String path) {

        int[] textureId = new int[1];
// Generate a texture object
        GLES20.glGenTextures(1, textureId, 0);
        int[] result = null;
        if (textureId[0] != 0) {
            InputStream is = Tools.readFromAsserts(path);
            Bitmap bitmap;
            try {
                bitmap = BitmapFactory.decodeStream(is);
            } finally {
                try {
                    is.close();
                } catch (IOException e) {
                    throw new RuntimeException("Error loading Bitmap.");
                }
            }
            result = new int[3];
            result[TEXTURE_ID] = textureId[0]; // TEXTURE_ID
            result[TEXTURE_WIDTH] = bitmap.getWidth(); // TEXTURE_WIDTH
            result[TEXTURE_HEIGHT] = bitmap.getHeight(); // TEXTURE_HEIGHT
// Bind to the texture in OpenGL
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId[0]);
// Set filtering
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_NEAREST);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_CLAMP_TO_EDGE);
// Load the bitmap into the bound texture.
            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
// Recycle the bitmap, since its data has been loaded into OpenGL.
            bitmap.recycle();
        } else {
            throw new RuntimeException("Error loading texture.");
        }
        return result;
    }
}
