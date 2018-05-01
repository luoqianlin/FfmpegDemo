package gl.tools;

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
}
