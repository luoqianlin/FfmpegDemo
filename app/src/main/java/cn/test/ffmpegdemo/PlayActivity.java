package cn.test.ffmpegdemo;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;

import com.sansi.va.VideoCodec;

import java.util.ArrayList;
import java.util.List;

public class PlayActivity extends AppCompatActivity {
    SurfaceView sv;
    VideoCodec videoCodec;
    ImageView imageView;

    List<Bitmap> bitmaps=new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play);
        sv=findViewById(R.id.sv);
        imageView=findViewById(R.id.iv);


        sv.getHolder().addCallback(new SurfaceHolder.Callback2() {
            @Override
            public void surfaceRedrawNeeded(SurfaceHolder holder) {

            }

            @Override
            public void surfaceCreated(final SurfaceHolder holder) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
//                        play(holder.getSurface());
                        videoCodec=new VideoCodec("/sdcard/Wildlife.wmv");
                        videoCodec.open();
                        int retV = videoCodec.init();
                        System.out.println("videoCodec ret:"+retV);
//                        Bitmap bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
//                        while (!videoCodec.fillBitmap(bitmap)){
//                            Canvas canvas = holder.lockCanvas();
//                            canvas.drawColor(Color.BLACK);
//                            canvas.drawBitmap(bitmap,0,0,null);
//                            holder.unlockCanvasAndPost(canvas);
////                            bitmap = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
//                        }

//                        while (!videoCodec.display(holder.getSurface()));
                        videoCodec.play(holder.getSurface());
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
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });



    }

    public native int play(Object surface);



}
