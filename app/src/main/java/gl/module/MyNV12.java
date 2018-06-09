package gl.module;

import android.opengl.GLES20;

import com.sansi.va.VAFrame;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import gl.context.MyGLNV12Context;

public class MyNV12 {

    MyGLNV12Context glyuvContext;

    float [] vertexData= {
                1.0f, -1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f
    };

    float []textureVertexData = {
        1.0f, 0.0f,//右下
                0.0f, 0.0f,//左下
                1.0f, 1.0f,//右上
                0.0f, 1.0f//左上
    };

    private final FloatBuffer mVertexBuffer;
    private final FloatBuffer mTexVertexBuffer;
//    private final ShortBuffer mVertexIndexBuffer;

    private VAFrame vaFrame;

    public MyNV12(MyGLNV12Context glyuvContext) {
        this.glyuvContext = glyuvContext;
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

    }

    public VAFrame getVaFrame() {
        return vaFrame;
    }

    synchronized public void setVaFrame(VAFrame vaFrame) {
        if (this.vaFrame != null) {
            vaFrame.destory();
        }
        this.vaFrame = vaFrame;
    }

    boolean isdraw=false;
    /**
     *
     *  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
     **/
    public synchronized void draw(){
        glyuvContext.useProgram();
        if (this.vaFrame == null) return;

        GLES20.glEnableVertexAttribArray(glyuvContext.getPositionHandle());
        GLES20.glVertexAttribPointer(glyuvContext.getPositionHandle(), 3, GLES20.GL_FLOAT, false,
                12, mVertexBuffer);

        GLES20.glEnableVertexAttribArray(glyuvContext.getTexCoordHandle());
        GLES20.glVertexAttribPointer(glyuvContext.getTexCoordHandle(), 2, GLES20.GL_FLOAT, false, 8,
                mTexVertexBuffer);

        int[] yuvTexId = glyuvContext.getNV12TexId();
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
                    vaFrame.getHeight()/2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vaFrame.getData()[1]);
//        }

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);

        GLES20.glDisableVertexAttribArray(glyuvContext.getTexCoordHandle());
        GLES20.glDisableVertexAttribArray(glyuvContext.getPositionHandle());
    }
}
