package com.sansi.va;

import java.nio.Buffer;

public class VAFrame {
    private int[] linesize;
    private int width, height;
    private Buffer[] data;

    private long ptr;

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

    public void destory(){
        if(ptr>0) {
            destory(this.ptr);
            ptr = -1;
        }
    }
    private native void destory(long ptr);

    public long getPtr() {
        return ptr;
    }
}
