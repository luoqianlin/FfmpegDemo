package cn.test.ffmpegdemo;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.sansi.va.VideoCodec;

import java.util.ArrayList;
import java.util.List;

public class PlayActivity extends AppCompatActivity {
    SurfaceView sv;
    VideoCodec videoCodec;
    SurfaceView sv1;
    VideoCodec videoCodec2;
    final static int FRAME_BUFFER_SIZE=30;
    int currentBufferSize=0;

    List<Frame> frames=new ArrayList<Frame>();
    List<Frame> c_frames=new ArrayList<>();



  class Frame{
      Bitmap bitmap;
      boolean isDrawed=false;
  }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play);
        sv=findViewById(R.id.sv);
        sv1 =findViewById(R.id.sv1);

        sv.getHolder().addCallback(new SurfaceHolder.Callback2() {
            @Override
            public void surfaceRedrawNeeded(SurfaceHolder holder) {

            }

            @Override
            public void surfaceCreated(final SurfaceHolder holder) {

            }

            @Override
            public void surfaceChanged(final SurfaceHolder holder, int format, final int width, final int height) {
                Log.e("PlayActivity","width:"+width+",height:"+height);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
//                        play(holder.getSurface());
                        videoCodec=new VideoCodec("/sdcard/Wildlife.wmv");
                        videoCodec.open();
                        int retV = videoCodec.init(width,height);
                        System.out.println("videoCodec ret:"+retV);
//                        Bitmap bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
//                        while (!videoCodec.fillBitmap(bitmap)){
//                            Canvas canvas = holder.lockCanvas();
////                            canvas.drawColor(Color.BLACK);
//                            Paint paint = new Paint();
//                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
//                            canvas.drawPaint(paint);
//                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
//                            canvas.drawBitmap(bitmap,0,0,null);
//                            holder.unlockCanvasAndPost(canvas);
////                            bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
//                        }
//                        SurfaceTexture surfaceTexture=new SurfaceTexture(1);

//                        while (!videoCodec.display(holder.getSurface()));
                        long start = System.currentTimeMillis();
                        videoCodec.play(holder.getSurface());
                        long end=System.currentTimeMillis();
                        System.out.println(String.format("播放耗时:%.2fs",(end-start)/1000.0));
                       /* byte[] frame = videoCodec.getNextFrame();
                        while (frame!=null) {


                            int[] colors = new int[frame.length];
                            for (int i = 0; i < frame.length; i++) {
                                colors[i] = frame[i] & 0xFF;
                                if(i<20) {
                                    System.out.printf("%02x ", colors[i]);
                                    if ((i+1) % 4 == 0) System.out.println();
                                }
                            }

//                            Bitmap bitmap= BitmapFactory.decodeResource(getResources(),R.mipmap.ic_launcher);
//                            runOnUiThread(new Runnable() {
//                                @Override
//                                public void run() {
//                                    imageView.setImageBitmap(bitmap);
////                                    imageView.setImageResource(R.mipmap.ic_launcher);
//                                }
//                            });
                            Canvas canvas = holder.lockCanvas();
                            canvas.drawColor(Color.BLACK);
                            canvas.drawBitmap(bitmap,0,0,null);
                            holder.unlockCanvasAndPost(canvas);
//                            try {
//                               Thread.sleep(2000);
//                            } catch (InterruptedException e) {
//                                e.printStackTrace();
//                            }
                            frame = videoCodec.getNextFrame();
                            System.out.println("frame data len:" + frame.length);
                        }*/
                        videoCodec.close();
                    }
                }).start();
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });


        sv1.getHolder().addCallback(new SurfaceHolder.Callback() {
            private volatile boolean isfinished;
            @Override
            public void surfaceCreated(SurfaceHolder holder) {

            }

            @Override
            public void surfaceChanged(final SurfaceHolder holder, final int format, final int width, final int height) {
               /* new Thread(new Runnable() {
                    @Override
                    public void run() {
                        videoCodec2=new VideoCodec("/sdcard/Wildlife.wmv");
                        videoCodec2.open();
                        int retV = videoCodec2.init();
                        System.out.println("videoCodec ret:"+retV);
                        Bitmap bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
                        boolean b = !videoCodec2.fillBitmap(bitmap);

                        while (b){
                            Canvas canvas = holder.lockCanvas();
                            long start=System.currentTimeMillis();
//                            canvas.drawColor(Color.BLACK);
                            Paint paint = new Paint();
                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
                            canvas.drawPaint(paint);
                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
                            canvas.drawBitmap(bitmap,0,0,null);
                            long end=System.currentTimeMillis();
                            Log.e("VideCodec","draw coast:"+(end-start));
                            holder.unlockCanvasAndPost(canvas);
//                            bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
                            b = !videoCodec2.fillBitmap(bitmap);
                            long end1=System.currentTimeMillis();
                            Log.e("VideCodec","get bitmap cost:"+(end1-end));
                        }

                        videoCodec2.close();
                    }
                }).start();*/

               Log.i("Main:","width:"+width+" height:"+height);

                new Thread(new Runnable() {



                    @Override
                    public void run() {
                        videoCodec2=new VideoCodec("/sdcard/Wildlife.wmv");
                        videoCodec2.open();
                        int retV = videoCodec2.init();
                        System.out.println("videoCodec ret:"+retV);
                        long startTime = System.currentTimeMillis();
                        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                        isfinished = videoCodec2.fillBitmap(bitmap);

                        Frame frame = new Frame();
                        frame.bitmap = bitmap;
                        frame.isDrawed = false;

                        while (!isfinished){

                            synchronized (frames){
                                frame.isDrawed=false;
                                while (currentBufferSize >= FRAME_BUFFER_SIZE) {
                                    try {
                                        Log.e("Main","currentBufferSize:"+currentBufferSize+",等待消费者消费");
                                        frames.wait();
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                                frames.add(frame);
                                currentBufferSize++;
                                frames.notify();
                            }

                            if (!c_frames.isEmpty()) {
                                frame = c_frames.remove(0);
                                frame.isDrawed = false;
                            } else {
                                frame = new Frame();
                                frame.isDrawed = false;
                                frame.bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                            }
                            isfinished = videoCodec2.fillBitmap(frame.bitmap);
                        }

                        long endTime = System.currentTimeMillis();
                        System.out.println(String.format("填充为bitmap耗时:%.2fs",(endTime-startTime)/1000.0));

                        videoCodec2.close();
                    }
                }).start();

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        while (true) {
                            if(isfinished)break;
                            synchronized (frames) {
                                while (currentBufferSize < 1 && !isfinished) {
                                    try {
                                        Log.e("Main","currentBufferSize:"+currentBufferSize+",等待生产者生产");
                                        frames.wait();
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                                for (Frame frame : frames) {
                                    if (!frame.isDrawed) {
                                        Canvas canvas = holder.lockCanvas();
                                        if(canvas!=null) {
                                            long start = System.currentTimeMillis();
                                            Paint paint = new Paint();
                                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
                                            canvas.drawPaint(paint);
                                            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
                                            canvas.drawBitmap(frame.bitmap, 0, 0, null);
                                            frame.isDrawed = true;
                                            holder.unlockCanvasAndPost(canvas);
                                            frames.remove(frame);
//                                            frame.bitmap.recycle();
                                            c_frames.add(frame);
                                            currentBufferSize--;
                                            frames.notify();
                                            break;
                                        }
                                    }
                                }

                            }
                        }
                    }
                }).start();
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });




    }

    public native int play(Object surface);





}
