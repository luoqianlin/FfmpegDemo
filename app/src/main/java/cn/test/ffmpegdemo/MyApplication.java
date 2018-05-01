package cn.test.ffmpegdemo;

import android.app.Application;
import android.util.DisplayMetrics;

import java.io.IOException;
import java.util.Arrays;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/4/23 10:15
 * 类说明
 */
public class MyApplication extends Application {
    public  static MyApplication application;
    @Override
    public void onCreate() {
        super.onCreate();

        application=this;
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float density = displayMetrics.density;
        int densityDpi = displayMetrics.densityDpi;
        int heightPixels = displayMetrics.heightPixels;
        int widthPixels = displayMetrics.widthPixels;
        System.out.println(String.format("widthPixels:%d,heightPixels:%d,density:%.2f,densityDpi:%d",
                widthPixels,heightPixels,density,densityDpi));

        String[] list = new String[0];
        try {
            list = MyApplication.getApplication().getAssets().list("/");
            System.out.println("===>"+ Arrays.toString(list));
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    public static MyApplication getApplication() {
        return application;
    }
}
