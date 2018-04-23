package cn.test.ffmpegdemo;

import android.app.Application;
import android.util.DisplayMetrics;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/4/23 10:15
 * 类说明
 */
public class MyApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float density = displayMetrics.density;
        int densityDpi = displayMetrics.densityDpi;
        int heightPixels = displayMetrics.heightPixels;
        int widthPixels = displayMetrics.widthPixels;
        System.out.println(String.format("widthPixels:%d,heightPixels:%d,density:%.2f,densityDpi:%d",
                widthPixels,heightPixels,density,densityDpi));
    }
}
