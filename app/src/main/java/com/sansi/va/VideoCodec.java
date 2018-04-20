package com.sansi.va;

import android.graphics.Bitmap;
import android.util.Log;
import android.view.Surface;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/4/20 10:32
 * 类说明
 */
public class VideoCodec {
    static {
        avInitialize();
    }

    long ptr;

    private String file;

    public VideoCodec(String file) {
        this.file = file;
    }

    private native long create();

    private native void free(long ptr);


    private native int init(long ptr,String file);


    private native byte[] getNextFrame(long ptr);

    private native static void avInitialize();

    private native  boolean display(long ptr,Surface surface);

    private native  int play(long ptr,Surface surface);

    private native  boolean fillBitmap(long ptr, Bitmap bitmap);

    public boolean display(Surface surface){
        return display(this.ptr,surface);
    }

    public int play(Surface surface){
        return play(this.ptr,surface);
    }

    public int init() {
        return init(this.ptr,this.file);
    }

    public String getFile() {
        return file;
    }

    public void open() {
        this.ptr = create();
    }

    public void close() {
        free(ptr);
        ptr = -1;
    }

    public byte[] getNextFrame() {
        Log.e("VideoCodec","Java getNextFrame invoked");
        return getNextFrame(this.ptr);
    }

    public boolean fillBitmap(Bitmap bitmap){
        return fillBitmap(this.ptr,bitmap);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (ptr > 0) {
            close();
        }
    }
}
