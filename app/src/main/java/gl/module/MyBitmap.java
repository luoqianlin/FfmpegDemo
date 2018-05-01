package gl.module;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;

import gl.context.MyGLContext2;

public class MyBitmap {

    private MyGLContext2 myGLContext2;

    private  int mTexName;

    private  final float[] VERTEX = {   // in counterclockwise order:
            1f, 0.5f, 0.0f,   // top right
            -1.0f, 0.5f, 0.0f,  // top left
            -1.0f, -0.5f, 0.0f, // bottom left
            1.0f, -0.5f, 0.0f,  // bottom right
    };
    private  final short[] VERTEX_INDEX = {
            0, 1, 2, 0, 2, 3
    };
    private  final float[] TEX_VERTEX = {   // in clockwise order:
            1f, 0,  // bottom right
            0, 0,  // bottom left
            0, 1f,  // top left
           1, 1,  // top right
    };

    int x;
    int y;

    private final FloatBuffer mVertexBuffer;
    private final FloatBuffer mTexVertexBuffer;
    private final ShortBuffer mVertexIndexBuffer;
    private Bitmap bitmap;

    public MyBitmap(int x, int y,
                    int winWidth,int winHeight,
                    Bitmap bitmap, MyGLContext2 myGLContext2) {
        System.out.println("构造函数");
        this.bitmap=bitmap;
        this.myGLContext2=myGLContext2;
        this.x = x;
        this.y = y;
        int width=bitmap.getWidth();
        int height=bitmap.getHeight();
        PointF pointF=new PointF();
        for (int i = 0; i < VERTEX.length; i += 3) {
            if(i==0) {
                toGLCoordinate(x + width, y, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==3){
                toGLCoordinate(x, y, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==6){
                toGLCoordinate(x, y+height, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==9){
                toGLCoordinate(x+width, y+height, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }
        }

        System.out.println("****>"+Arrays.toString(VERTEX));
        mVertexBuffer = ByteBuffer.allocateDirect(VERTEX.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(VERTEX);
        mVertexBuffer.flip();

        mVertexIndexBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer()
                .put(VERTEX_INDEX);
        mVertexIndexBuffer.flip();

        mTexVertexBuffer = ByteBuffer.allocateDirect(TEX_VERTEX.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(TEX_VERTEX);
        mTexVertexBuffer.flip();
    }

    public void toGLCoordinate(int x, int y, int winWidth, int winHeight,PointF pointF) {
        pointF.x=2.0f * x / winWidth - 1.0f;
        pointF.y=1.0f - (2.0f * y / winHeight);
    }

    public MyBitmap(Bitmap bitmap, MyGLContext2 myGLContext2) {
        this.bitmap=bitmap;
        this.myGLContext2=myGLContext2;
        mVertexBuffer = ByteBuffer.allocateDirect(VERTEX.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(VERTEX);
        mVertexBuffer.flip();

        mVertexIndexBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer()
                .put(VERTEX_INDEX);
        mVertexIndexBuffer.flip();

        mTexVertexBuffer = ByteBuffer.allocateDirect(TEX_VERTEX.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(TEX_VERTEX);
        mTexVertexBuffer.flip();


//        bitmap.recycle();
    }

    public void draw(){
        myGLContext2.useProgram();


        GLES20.glEnableVertexAttribArray(myGLContext2.getPositionHandle());
        GLES20.glVertexAttribPointer(myGLContext2.getPositionHandle(), 3, GLES20.GL_FLOAT, false,
                12, mVertexBuffer);

        GLES20.glEnableVertexAttribArray(myGLContext2.getTexCoordHandle());
        GLES20.glVertexAttribPointer(myGLContext2.getTexCoordHandle(), 2, GLES20.GL_FLOAT, false, 0,
                mTexVertexBuffer);

        int[] texNames = myGLContext2.getTexNames();
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texNames[0]);
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);

        // 用 glDrawElements 来绘制，mVertexIndexBuffer 指定了顶点绘制顺序
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, VERTEX_INDEX.length,GLES20.GL_UNSIGNED_SHORT,
                mVertexIndexBuffer);

        GLES20.glDisableVertexAttribArray(myGLContext2.getTexCoordHandle());
        GLES20.glDisableVertexAttribArray(myGLContext2.getPositionHandle());

    }

    void destroy() {
        GLES20.glDeleteTextures(1, new int[] { mTexName }, 0);
    }
}
