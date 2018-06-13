package gl.module;

import android.graphics.PointF;
import android.opengl.GLES20;

import com.sansi.va.VAFrame;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;

import gl.context.MyGLContext2;

public class GL_RGB24 {

    private MyGLContext2 myGLContext2;

    private  int mTexName;
    int targetWidth;
    int targetHeight;

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
    int winWidth;
    int winHeight;
    private final FloatBuffer mVertexBuffer;
    private final FloatBuffer mTexVertexBuffer;
    private final ShortBuffer mVertexIndexBuffer;
    private  int deltaX=10;

    private VAFrame vaFrame;

    public void move(){
        setX(this.x+this.deltaX);
    }

    public void check(){
        if (deltaX > 0) {
            if (this.x+this.targetWidth >= this.winWidth) {
                this.deltaX = -1 * this.deltaX;
            }
        } else if(this.deltaX<0){
            if (this.x <= 0) {
                this.deltaX = -1 * this.deltaX;
            }
        }
        move();
    }

    public synchronized void setX(int x){
        this.x=x;
        int width=this.targetWidth;
        int height=this.targetHeight;
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

//        System.out.println("****>"+Arrays.toString(VERTEX));
        mVertexBuffer.clear();
        mVertexBuffer.put(VERTEX);
        mVertexBuffer.flip();
    }

    public void setVaFrame(VAFrame vaFrame) {
        if (this.vaFrame != null && this.vaFrame != vaFrame) {
            this.vaFrame.destory();
        }
        this.vaFrame = vaFrame;
    }

    public int getX() {
        return x;
    }

    public int getDeltaX() {
        return deltaX;
    }

    public int getWinWidth() {
        return winWidth;
    }

    public int getWinHeight() {
        return winHeight;
    }

    public GL_RGB24(int x, int y,
                    int winWidth, int winHeight,
                    int targetWidth, int targetHeight,
                    MyGLContext2 myGLContext2) {
        System.out.println("构造函数");
        this.myGLContext2=myGLContext2;
        this.winHeight=winHeight;
        this.winWidth=winWidth;
        this.targetWidth=targetWidth;
        this.targetHeight=targetHeight;
        this.x = x;
        this.y = y;
        PointF pointF=new PointF();
        for (int i = 0; i < VERTEX.length; i += 3) {
            if(i==0) {
                toGLCoordinate(x + targetWidth, y, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==3){
                toGLCoordinate(x, y, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==6){
                toGLCoordinate(x, y+ targetHeight, winWidth, winHeight, pointF);
                VERTEX[i] = pointF.x;
                VERTEX[i + 1] = pointF.y;
            }else if(i==9){
                toGLCoordinate(x+ targetWidth, y+ targetHeight, winWidth, winHeight, pointF);
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


    public synchronized void draw(){
        if (vaFrame == null) return;
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
//        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
        System.out.println("linesize:"+vaFrame.getLinesize()[0]
                +" height:"+vaFrame.getHeight()+",width:"+vaFrame.getWidth());
        System.out.println("winWidth:"+this.winWidth+",winHeight:"+this.winHeight
        +",taretWidht:"+this.targetWidth+",targetHeight:"+this.targetHeight+",data:"+vaFrame.getData()[0].limit());
        for(int i=0;i<vaFrame.getData().length;i++){
            if(vaFrame.getData()[i]!=null)
            System.out.println("["+i+"]:"+vaFrame.getData()[i].limit());
        }
        //    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 48, 48, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) 0x41cedb18);
//        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_RGBA,
                vaFrame.getWidth(),vaFrame.getHeight(),
                0,GLES20.GL_RGBA,GLES20.GL_UNSIGNED_BYTE,vaFrame.getData()[0]);
        // 用 glDrawElements 来绘制，mVertexIndexBuffer 指定了顶点绘制顺序
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, VERTEX_INDEX.length,GLES20.GL_UNSIGNED_SHORT,
                mVertexIndexBuffer);
//        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 6);

        GLES20.glDisableVertexAttribArray(myGLContext2.getTexCoordHandle());
        GLES20.glDisableVertexAttribArray(myGLContext2.getPositionHandle());

    }

    void destroy() {
        GLES20.glDeleteTextures(1, new int[] { mTexName }, 0);
    }
}
