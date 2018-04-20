package cn.test.ffmpegdemo;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    String videoPath = "/storage/emulated/0/360/80s.mp4";
    public SurfaceHolder surfaceHolder;

    public VideoSurfaceView(Context context) {
        super(context);
        init();
    }

    public VideoSurfaceView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init();
    }

    private void init() {
        surfaceHolder = getHolder();
        surfaceHolder.addCallback(this);
    }

    public void surfaceCreated(SurfaceHolder holder) {
    }

    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        Thread thread = new Thread() {
            @Override
            public void run() {
                super.run();
                videoPlay(videoPath, surfaceHolder.getSurface());
            }
        };
        thread.start();
    }

    public native void videoPlay(String path, Surface surface);
}