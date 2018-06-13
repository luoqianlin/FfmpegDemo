package gl.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.os.Handler;

import com.sansi.va.VAFrame;
import com.sansi.va.VideoCodec;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import cn.test.ffmpegdemo.MyApplication;
import gl.context.MyGLContext2;
import gl.context.MyGLNV12Context;
import gl.context.MyGLYUVContext;
import gl.module.GL_RGB24;
import gl.module.MyBitmap;
import gl.module.MyNV12;
import gl.module.MyYUV;
import cn.test.ffmpegdemo.R;

public class MyRenderer3 implements GLSurfaceView.Renderer {

    MyYUV myYUV;
    MyNV12 myNV12;
    MyGLYUVContext myGLYUVContext;
    GL_RGB24 glRgb24;
    MyGLNV12Context myGLNV12Context;


    Handler handler;
    private VideoCodec videoCodec;
    GLSurfaceView glSurfaceView;

    MyBitmap myBitmap;
    private MyGLContext2 glContext2;
    long startTime;
     int mWidth;
    int mHeight;
    private String  file = Environment.getExternalStorageDirectory()+"/VisualArts/materials/1d01d7f5180f7a03971861745130af11.avi";

    public MyRenderer3(GLSurfaceView glSurfaceView) {
        this.glSurfaceView=glSurfaceView;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glContext2=new MyGLContext2();

    }

    @Override
    public void onSurfaceChanged(GL10 gl, final int width, final int height) {
        System.out.println("mHeight:" + height);
        this.mHeight =height;
        this.mWidth =width;
        GLES20.glViewport(0, 0, width, height);
        Bitmap bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
        System.out.println("===========onSurfaceChanged===========");
//      Bitmap bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
//      myBitmap=new MyBitmap(mWidth/2-bitmap.getWidth()/2,mHeight/2-bitmap.getHeight()/2,mWidth,mHeight,bitmap,glContext2);
        myGLYUVContext = new MyGLYUVContext();
        myYUV = new MyYUV(0,0,width,height,width/2,height/2,myGLYUVContext);
        this.glRgb24=new GL_RGB24(0,0,width,height,width/2,height/2,glContext2);
        videoCodec = new VideoCodec(file);
        videoCodec.open();
        int retV = videoCodec.init(width, height);
        System.out.println("videoCodec ret:" + retV);
    }
/**
 * GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT
 * */
boolean first = false;
    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        long decodestart=System.currentTimeMillis();
        VAFrame vaFrame = videoCodec.nextFrame();
//        boolean ft = videoCodec.fillBitmap(myBitmap.getBitmap());
        long decodeend=System.currentTimeMillis();
//        System.out.printf("decode cost:%d\n",(decodeend-decodestart));
        if (vaFrame != null) {
//            System.out.println("---");
//                        System.out.println("解码出的视屏 mWidth:" + vaFrame.getWidth() + ",mHeight:" + vaFrame.getHeight());
//                        for(int i=0;i<3;i++){
//                            System.out.println("linesize["+i+"]="+vaFrame.getLinesize()[i]);
//                        }

//                    System.out.println("ptr:" + Long.toHexString(vaFrame.getPtr()));

            long renderstart=System.currentTimeMillis();
            if (vaFrame.getFormat() == 0) {
                myYUV.setVaFrame(vaFrame);
                myYUV.draw();
            } else {
                glRgb24.setVaFrame(vaFrame);
                glRgb24.draw();
            }
//            myBitmap.draw();
            long renderend=System.currentTimeMillis();
//            System.out.printf("render cost:%d\n",(renderend-renderstart));

        }else{
//            System.out.println("is null end");
            if(!first) {
                System.out.println("cost:" + (System.currentTimeMillis() - startTime) / 1000.0 + "s");
                first = true;
            }
            videoCodec.close();


            videoCodec = new VideoCodec(file);
            videoCodec.open();
            int retV = videoCodec.init(this.mWidth, this.mHeight);
            System.out.println("videoCodec ret:" + retV);
        }



    }
}