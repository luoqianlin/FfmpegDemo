package cn.test.ffmpegdemo;

import android.content.Context;
import android.os.Build;
import android.os.Environment;
import android.support.annotation.RequiresApi;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    String videoPath = Environment.getExternalStorageDirectory()
            +"/VisualArts/materials/b888d0a275c1eedaf0e80eb071d2fb47.avi";
    public SurfaceHolder surfaceHolder;

    public VideoSurfaceView(Context context) {
        super(context);
        init();
    }

    public VideoSurfaceView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init();
    }

    public VideoSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public VideoSurfaceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
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
                Log.e("Videosurface:",videoPath);
                videoPlay(videoPath, surfaceHolder.getSurface());
            }
        };
        thread.start();
    }

    public native void videoPlay(String path, Surface surface);
}