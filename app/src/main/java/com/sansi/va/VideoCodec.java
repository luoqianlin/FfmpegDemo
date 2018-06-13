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
        System.loadLibrary("native-lib");
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
    private native int init(long ptr,String file,int videoWidth,int videoHeight);


    private native byte[] getNextFrame(long ptr);

    private native VAFrame nextFrame(long ptr);

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
        return init(this.ptr, this.file);
    }

    public int init(int videoWidth, int videoHeight) {
        return init(this.ptr, this.file, videoWidth, videoHeight);
    }

    public String getFile() {
        return file;
    }

    public void open() {
        this.ptr = create();
    }

    public void close() {
        if (ptr != 0) {
            free(ptr);
            ptr = 0L;
        }
    }

    public byte[] getNextFrame() {
        Log.e("VideoCodec","Java getNextFrame invoked");
        return getNextFrame(this.ptr);
    }

    public VAFrame nextFrame(){
        return nextFrame(this.ptr);
    }

    public boolean fillBitmap(Bitmap bitmap){
        return fillBitmap(this.ptr,bitmap);
    }

    @Override
    protected void finalize() throws Throwable {
        try{
            if (ptr != 0) {
                close();
            }
        }finally {
            super.finalize();
        }


    }

    private native  int getVideoRawHeight(long ptr);
    private native  int getVideoRawWidth(long ptr);

    public int getVideoRawHeight(){
        return getVideoRawHeight(this.ptr);
    }

    public int getVideoRawWidth(){
        return getVideoRawWidth(this.ptr);
    }
}
