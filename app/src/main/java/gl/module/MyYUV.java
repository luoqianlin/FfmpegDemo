package gl.module;

import android.opengl.GLES20;

import com.sansi.va.VAFrame;

import gl.context.MyGLYUVContext;


public class MyYUV  extends VBO{
    MyGLYUVContext glYuvTexturePaint;
    private VAFrame vaFrame;

    public MyYUV(int x, int y, int winWidth, int winHeight,
                 int targetWidth, int targetHeight, MyGLYUVContext glYuvTexturePaint) {
        super(x,y,winWidth,winHeight,targetWidth,targetHeight);
        System.out.println("构造函数");
        this.glYuvTexturePaint = glYuvTexturePaint;
    }

    public VAFrame getVaFrame() {
        return vaFrame;
    }

    synchronized public void setVaFrame(VAFrame vaFrame) {
        if (this.vaFrame != null && this.vaFrame != vaFrame) {
            this.vaFrame.destory();
        }
        this.vaFrame = vaFrame;
        setDisplyType(this.vaFrame.getWidth(),this.vaFrame.getHeight(), VBO.ASPECT_RATIO);
    }

    boolean isdraw=false;
    /**
     *
     *  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
     **/
    public synchronized void draw(){
        if (this.vaFrame == null || this.vaFrame.isDestory()) return;
        glYuvTexturePaint.useProgram();
        GLES20.glEnableVertexAttribArray(glYuvTexturePaint.getPositionHandle());
        GLES20.glVertexAttribPointer(glYuvTexturePaint.getPositionHandle(), 3, GLES20.GL_FLOAT, false,
                12, mVertexBuffer);

        GLES20.glEnableVertexAttribArray(glYuvTexturePaint.getTexCoordHandle());
        GLES20.glVertexAttribPointer(glYuvTexturePaint.getTexCoordHandle(), 2, GLES20.GL_FLOAT, false, 8,
                mTexVertexBuffer);

        int[] yuvTexId = glYuvTexturePaint.getYuvTexId();
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[0]);
//        if(isdraw) {
//            GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[0],
//                    vaFrame.getHeight(), 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[0]);
//        }else{
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[0],
                vaFrame.getHeight(), 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[0]);
//        }

        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[1]);
//        if(isdraw) {
//            GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[1],
//                    vaFrame.getHeight() / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[1]);
//        }else{
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[1],
                vaFrame.getHeight() / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[1]);
//        }

        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, yuvTexId[2]);
//        if(isdraw) {
//            GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[2],
//                    vaFrame.getHeight() / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[2]);
//            isdraw=true;
//        }else{
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, vaFrame.getLinesize()[2],
                vaFrame.getHeight() / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[2]);
//        }
//        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);
        GLES20.glDrawElements(GLES20.GL_TRIANGLE_STRIP, VERTEX_INDEX.length,GLES20.GL_UNSIGNED_SHORT,
                mVertexIndexBuffer);
        GLES20.glDisableVertexAttribArray(glYuvTexturePaint.getTexCoordHandle());
        GLES20.glDisableVertexAttribArray(glYuvTexturePaint.getPositionHandle());
    }
}