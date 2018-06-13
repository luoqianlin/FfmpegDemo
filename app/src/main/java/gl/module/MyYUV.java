package gl.module;

import android.graphics.PointF;
import android.opengl.GLES20;

import com.sansi.va.VAFrame;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;

import gl.context.MyGLYUVContext;
import gl.tools.Tools;


public class MyYUV {

    MyGLYUVContext glYuvTexturePaint;

    float [] vertexData= {
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f
    };

    float []textureVertexData = {
            /*    1.0f, 0.0f,//右下
                        0.0f, 0.0f,//左下
                        1.0f, 1.0f,//右上
                        0.0f, 1.0f//左上*/

            1f, 0,  // bottom right
            0, 0,  // bottom left
            0, 1f,  // top left
            1, 1,  // top right
    };

    private  FloatBuffer mVertexBuffer;
    private final FloatBuffer mTexVertexBuffer;
    private final ShortBuffer mVertexIndexBuffer;

    private VAFrame vaFrame;
    private int winHeight;
    private  int winWidth;
    private int x;
    private  int y;
    private  final short[] VERTEX_INDEX = {
            0, 1, 2, 0, 2, 3
    };
    private int width;
    private int height;

    public MyYUV(MyGLYUVContext glYuvTexturePaint) {
        this.glYuvTexturePaint = glYuvTexturePaint;
        mVertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        mVertexBuffer.flip();

        mTexVertexBuffer = ByteBuffer.allocateDirect(textureVertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureVertexData);
        mTexVertexBuffer.flip();

        mVertexIndexBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer()
                .put(VERTEX_INDEX);
        mVertexIndexBuffer.flip();

    }


    public MyYUV(int x, int y, int winWidth, int winHeight,
                 int targetWidth, int targetHeight, MyGLYUVContext glYuvTexturePaint) {
        System.out.println("构造函数");
        this.glYuvTexturePaint = glYuvTexturePaint;
        this.winHeight = winHeight;
        this.winWidth = winWidth;
        setRect(x, y, targetWidth, targetHeight);
        mTexVertexBuffer = ByteBuffer.allocateDirect(textureVertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureVertexData);
        mTexVertexBuffer.flip();

        mVertexIndexBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer()
                .put(VERTEX_INDEX);
        mVertexIndexBuffer.flip();
    }

    private void setRect(int x, int y, int targetWidth, int targetHeight) {
        this.width = targetWidth;
        this.height = targetHeight;
        this.x = x;
        this.y = y;
        PointF pointF=new PointF();
        for (int i = 0; i < vertexData.length; i += 3) {
            if (i == 0) {
                Tools.toGLCoordinate(x + width, y, this.winWidth, this.winHeight, pointF);
                vertexData[i] = pointF.x;
                vertexData[i + 1] = pointF.y;
            } else if (i == 3) {
                Tools.toGLCoordinate(x, y, this.winWidth, this.winHeight, pointF);
                vertexData[i] = pointF.x;
                vertexData[i + 1] = pointF.y;
            } else if (i == 6) {
                Tools.toGLCoordinate(x, y + height, this.winWidth, this.winHeight, pointF);
                vertexData[i] = pointF.x;
                vertexData[i + 1] = pointF.y;
            } else if (i == 9) {
                Tools.toGLCoordinate(x + width, y + height, this.winWidth, this.winHeight, pointF);
                vertexData[i] = pointF.x;
                vertexData[i + 1] = pointF.y;
            }
        }
        System.out.println("****>"+ Arrays.toString(vertexData));
        if(mVertexBuffer==null) {
            mVertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    .put(vertexData);
            mVertexBuffer.flip();
        }else{
            mVertexBuffer.clear();
            mVertexBuffer.put(vertexData);
            mVertexBuffer.flip();
        }

    }


    public VAFrame getVaFrame() {
        return vaFrame;
    }

    synchronized public void setVaFrame(VAFrame vaFrame) {
        if (this.vaFrame != null && this.vaFrame != vaFrame) {
            this.vaFrame.destory();
        }
        this.vaFrame = vaFrame;
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