package gl.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;

import java.io.IOException;
import java.io.InputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import gl.context.MyGLContext;
import gl.context.MyGLContext2;
import gl.module.MyBitmap;
import gl.module.MyTriangle;
import gl.tools.Tools;

public class MyRenderer implements GLSurfaceView.Renderer {

    MyTriangle myTriangle;
    MyTriangle myTriangle2;
    MyBitmap myBitmap;
    private MyGLContext glContext;
    private MyGLContext2 glContext2;

    Handler handler=new Handler();

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glContext = new MyGLContext();

        glContext2=new MyGLContext2();
        float[] vertex = {   // in counterclockwise order:
                0, 1, 0,  // top
                -0.5f, -1, 0,  // bottom left
                1, -1, 0,  // bottom right
        };

        myTriangle = new MyTriangle(glContext,vertex);

        float[] vertex2 = {   // in counterclockwise order:
                -0.5f, 0.5f, 0,  // top
                -1f, -0f, 0,  // bottom left
                0, 0, 0,  // bottom right
        };
        myTriangle2 = new MyTriangle(glContext,vertex2);
        myTriangle2.setColor(new float[]{255.0f, 0, 0, 1f});

//        Bitmap bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
        InputStream is = Tools.readFromAsserts("ic_launcher.png");
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



    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        System.out.println("mHeight:"+height);
        GLES20.glViewport(0, 0, width, height);
        InputStream is = Tools.readFromAsserts("ic_launcher.png");
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
        System.out.println("===========onSurfaceChanged===========");
//                Bitmap bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
//        myBitmap=new MyBitmap(mWidth/2-bitmap.getWidth()/2,mHeight/2-bitmap.getHeight()/2,mWidth,mHeight,bitmap,glContext2);
        myBitmap=new MyBitmap(0,0,width,height,bitmap.getWidth(),bitmap.getHeight(),bitmap,glContext2,MyBitmap.FIT_XY);
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                synchronized (myTriangle2) {
                    float[] vertex = myTriangle2.getVertex();
                    float maxVer=-1.0f;
                    float minVer=1.0f;

                    for (int i = 0; i < vertex.length; i++) {
                        if (i % 3 == 0) {
                            maxVer=Math.max(maxVer,vertex[i]);
                            minVer=Math.min(minVer,vertex[i]);
                        }
                    }

//                    System.out.println("vertex[i]:"+vertex[i]);
                    if (Float.compare(maxVer, 1.0f) >= 0) {
                        if(myTriangle2.getDeltaX()>0){
                            myTriangle2.setDeltaX(-1*myTriangle2.getDeltaX());
                        }
                    }else if(Float.compare(minVer,-1.0f)<=0){
                        if(myTriangle2.getDeltaX()<0){
                            myTriangle2.setDeltaX(-1*myTriangle2.getDeltaX());
                        }
                    }

                    for (int i = 0; i < vertex.length; i++) {
                        if (i % 3 == 0) {
                            vertex[i] += myTriangle2.getDeltaX();
                        }
                    }

                    myTriangle2.setVertex(vertex);

                    handler.postDelayed(this,100L);
                }
            }
        },100L);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        myTriangle.draw( gl);
        if(myBitmap!=null) {
            myBitmap.draw();
        }
        synchronized (myTriangle2) {
            myTriangle2.draw(gl);
        }

    }
}