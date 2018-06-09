package gl.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;

import com.sansi.va.VAFrame;
import com.sansi.va.VideoCodec;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import cn.test.ffmpegdemo.MyApplication;
import cn.test.ffmpegdemo.R;
import gl.context.MyGLContext2;
import gl.context.MyGLNV12Context;
import gl.context.MyGLYUVContext;
import gl.module.MyBitmap;
import gl.module.MyNV12;
import gl.module.MyYUV;

public class MyRenderer3 implements GLSurfaceView.Renderer {

    MyYUV myYUV;
    MyNV12 myNV12;
    MyGLYUVContext myGLYUVContext;
    MyGLNV12Context myGLNV12Context;


    Handler handler;
    private VideoCodec videoCodec;
    GLSurfaceView glSurfaceView;

    MyBitmap myBitmap;
    private MyGLContext2 glContext2;
    private final HandlerThread handlerThread;
    private Runnable decodeHandler;
//    private Runnable closeHandler;
    long startTime;
     int width;
    int height;
    private String  file = Environment.getExternalStorageDirectory()+"/VisualArts/materials/6b97f7d5fea76ef1db59b36d8eba7bea.mkv";

    public MyRenderer3(GLSurfaceView glSurfaceView) {
        this.glSurfaceView=glSurfaceView;

        handlerThread = new HandlerThread("xxxx");
        handlerThread.start();
        handler = new Handler(handlerThread.getLooper());
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glContext2=new MyGLContext2();

    }

    @Override
    public void onSurfaceChanged(GL10 gl, final int width, final int height) {
        System.out.println("height:" + height);
        this.height=height;
        this.width=width;
        GLES20.glViewport(0, 0, width, height);
//        InputStream is = Tools.readFromAsserts("ic_launcher.png");
        Bitmap bitmap;
//        try {
            bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
//        } finally {
//            try {
//                is.close();
//            } catch (IOException e) {
//                throw new RuntimeException("Error loading Bitmap.");
//            }
//        }
        System.out.println("===========onSurfaceChanged===========");
//                Bitmap bitmap = BitmapFactory.decodeResource(MyApplication.getApplication().getResources(), R.mipmap.ic_launcher);
//        myBitmap=new MyBitmap(width/2-bitmap.getWidth()/2,height/2-bitmap.getHeight()/2,width,height,bitmap,glContext2);
        myBitmap=new MyBitmap(width/2-bitmap.getWidth()/2,height/2-bitmap.getHeight()/2,width,height,bitmap,glContext2);
        myGLYUVContext = new MyGLYUVContext();
        myGLNV12Context =new MyGLNV12Context();
        myNV12=new MyNV12(myGLNV12Context);
        myYUV = new MyYUV(myGLYUVContext);
        videoCodec = new VideoCodec(file);
        videoCodec.open();
        int retV = videoCodec.init(width, height);
        System.out.println("videoCodec ret:" + retV);
//        long start = System.currentTimeMillis();
//        videoCodec.play(holder.getSurface());
//        long end=System.currentTimeMillis();
//        System.out.println(String.format("播放耗时:%.2fs",(end-start)/1000.0));
        //                        System.out.println("解码出的视屏 width:" + vaFrame.getWidth() + ",height:" + vaFrame.getHeight());
//                        for(int i=0;i<3;i++){
//                            System.out.println("linesize["+i+"]="+vaFrame.getLinesize()[i]);
//                        }
//                    System.out.println("ptr:" + Long.toHexString(vaFrame.getPtr()));
//                    glSurfaceView.queueEvent(new Runnable() {
//                        @Override
//                        public void run() {
//                        }
//                    });
//                    glContext2.deleteProgram();
//                    myGLYUVContext.deleteProgram();
//                    handlerThread.quit();


        decodeHandler = new Runnable() {
              @Override
              public void run() {

                  final VAFrame vaFrame = videoCodec.nextFrame();
                  if (vaFrame != null) {
  //                        System.out.println("解码出的视屏 width:" + vaFrame.getWidth() + ",height:" + vaFrame.getHeight());
  //                        for(int i=0;i<3;i++){
  //                            System.out.println("linesize["+i+"]="+vaFrame.getLinesize()[i]);
  //                        }

  //                    System.out.println("ptr:" + Long.toHexString(vaFrame.getPtr()));

  //                    glSurfaceView.queueEvent(new Runnable() {
  //                        @Override
  //                        public void run() {
                      myYUV.setVaFrame(vaFrame);
                      myBitmap.check();

                      glSurfaceView.requestRender();
  //                        }
  //                    });
                      handler.postDelayed(this, 10);

                  } else {
  //                    glContext2.deleteProgram();
  //                    myGLYUVContext.deleteProgram();
                      System.out.println("cost:"+(System.currentTimeMillis()-startTime)/1000.0+"s");
                      videoCodec.close();
                      videoCodec.open();
                      int retV = videoCodec.init(width, height);
                      System.out.println("videoCodec ret:" + retV);
                      handler.postDelayed(decodeHandler, 1000);
                      startTime=System.currentTimeMillis();


  //                    handlerThread.quit();
                  }
              }
          };
//        closeHandler = new Runnable() {
//            @Override
//            public void run() {
//                videoCodec.close();
//                videoCodec.open();
//                int retV = videoCodec.init(width, height);
//                handler.postDelayed(decodeHandler, 10);
//            }
//        };
        startTime=System.currentTimeMillis();
//      handler.postDelayed(decodeHandler, 10);
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
        long decodeend=System.currentTimeMillis();
//        System.out.printf("decode cost:%d\n",(decodeend-decodestart));
        if (vaFrame != null) {
//            System.out.println("---");
//                        System.out.println("解码出的视屏 width:" + vaFrame.getWidth() + ",height:" + vaFrame.getHeight());
//                        for(int i=0;i<3;i++){
//                            System.out.println("linesize["+i+"]="+vaFrame.getLinesize()[i]);
//                        }

//                    System.out.println("ptr:" + Long.toHexString(vaFrame.getPtr()));

            long renderstart=System.currentTimeMillis();
            myYUV.setVaFrame(vaFrame);
//            handler.postDelayed(this, 100L);
//            myBitmap.check();

            if (myYUV != null) {
//            synchronized (myYUV) {
                myYUV.draw();
//            }
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
            int retV = videoCodec.init(width, height);
            System.out.println("videoCodec ret:" + retV);
        }



    }
}