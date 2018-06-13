package gl.tools;

import android.graphics.PointF;

import com.google.common.io.ByteStreams;

import java.io.IOException;
import java.io.InputStream;

import cn.test.ffmpegdemo.MyApplication;

public class Tools {


    public static String readFromAssets(String s) {

        try {
            return new String(ByteStreams.toByteArray(MyApplication.getApplication().getAssets().open(s)),"UTF-8");
        } catch (IOException e) {
           throw  new RuntimeException(e);
        }
    }

    public static InputStream readFromAsserts(String path) {
        try {

            return MyApplication.getApplication().getAssets().open(path);
        } catch (IOException e) {
            throw  new RuntimeException(e);
        }
    }
    public static void toGLCoordinate(int x, int y, int winWidth, int winHeight,PointF pointF) {
        pointF.x = 2.0f * x / winWidth - 1.0f;
        pointF.y = 1.0f - (2.0f * y / winHeight);
    }
}
