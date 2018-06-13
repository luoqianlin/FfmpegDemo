package com.sansi.va;

import android.graphics.Bitmap;

import java.nio.Buffer;

public class VAFrame {
    private int[] linesize;
    private int width, height;
    private Buffer[] data;
    private Bitmap bitmap;
    private int format;

    private long ptr=0;

    public int[] getLinesize() {
        return linesize;
    }

    public void setLinesize(int[] linesize) {
        this.linesize = linesize;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public Buffer[] getData() {
        return data;
    }

    public void setData(Buffer[] data) {
        this.data = data;
    }

    public int getFormat() {
        return format;
    }

    public void setFormat(int format) {
        this.format = format;
    }

    public void destory() {
        if (ptr != 0) {
            destory(this.ptr);
            ptr = 0;
        }
    }

    public synchronized boolean isDestory() {
        return ptr == 0;
    }

    private native void destory(long ptr);

    public long getPtr() {
        return ptr;
    }
}
