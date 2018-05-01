package gl.test;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import cn.test.ffmpegdemo.R;

public class BitmapActivity extends AppCompatActivity {
    GLSurfaceView sv;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bitmap);
        sv=findViewById(R.id.sv);
        sv.setEGLContextClientVersion(2);
        sv.setEGLConfigChooser(new MyConfigChooser());  //一定要在setRender之前调用
        sv.setRenderer(new MyRenderer3(sv));
        sv.setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR);
//        sv.setRenderMode(RENDERMODE_WHEN_DIRTY);
    }

    @Override
    protected void onResume() {
        super.onResume();
        sv.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        sv.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }


}
