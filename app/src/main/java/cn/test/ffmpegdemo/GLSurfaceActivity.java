package cn.test.ffmpegdemo;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLSurfaceActivity extends AppCompatActivity {
GLSurfaceView glsv;
    private Triangle mTriangle;
    private Square mSquare;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_glsurface);
       glsv=findViewById(R.id.glsv);
        //GLContext设置为OpenGLES2.0
        glsv.setEGLContextClientVersion(2);
        //在setRenderer之前，可以调用以下方法来进行EGL设置
        //mGLView.setEGLConfigChooser();    //颜色、深度、模板等等设置
        //mGLView.setEGLWindowSurfaceFactory(); //窗口设置
        //mGLView.setEGLContextFactory();   //EGLContext设置
        //设置渲染器，渲染主要就是由渲染器来决定
        glsv.setRenderer(new GLSurfaceView.Renderer() {
            // mMVPMatrix is an abbreviation for "Model View Projection Matrix"
            private final float[] mMVPMatrix = new float[16];
            private final float[] mProjectionMatrix = new float[16];
            private final float[] mViewMatrix = new float[16];

            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                // 初始化triangle
                mTriangle = new Triangle();
                // 初始化 square
                mSquare = new Square();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                GLES20.glViewport(0, 0, width, height);
                float ratio = (float) width / height;

                // this projection matrix is applied to object coordinates
                // in the onDrawFrame() method
                Matrix.frustumM(mProjectionMatrix, 0, -ratio, ratio, -1, 1, 3, 7);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

                // Set the camera position (View matrix)
                Matrix.setLookAtM(mViewMatrix, 0, 0, 0, -3, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

                // Calculate the projection and view transformation
                Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0);
                mTriangle.draw(mMVPMatrix);
            }
        });
        /*渲染方式，RENDERMODE_WHEN_DIRTY表示被动渲染，只有在调用requestRender或者onResume等方法时才会进行渲染。RENDERMODE_CONTINUOUSLY表示持续渲染*/
//        glsv.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

}
