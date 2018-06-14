package gl.module;

import android.graphics.Bitmap;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import gl.context.MyGLContext2;

public class MyBitmap  extends VBO{

    private MyGLContext2 myGLContext2;

    private  int mTexName;


    private final Bitmap bitmap;
    private  int deltaX=3;

    public void move(){
        setX(this.x+this.deltaX);
    }

    public void check(){
        if (deltaX > 0) {
            if (this.x+this.targetWidth>= this.winWidth) {
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
        this.x = x;
        invalidate();
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

    public Bitmap getBitmap() {
        return bitmap;
    }




    public MyBitmap(int x, int y,
                    int winWidth, int winHeight,
                    int targetWidth, int targetHeight,
                    Bitmap bitmap, MyGLContext2 myGLContext2,int displyType) {
        super(x,y,winWidth,winHeight,targetWidth,targetHeight);
        System.out.println("构造函数");
        this.bitmap=bitmap;
        this.myGLContext2=myGLContext2;
        setDisplyType(bitmap.getWidth(),bitmap.getHeight(),displyType);
    }


    public MyBitmap(Bitmap bitmap, MyGLContext2 myGLContext2) {
       this(0,0,bitmap.getWidth(),bitmap.getHeight(),
               bitmap.getWidth(),bitmap.getHeight(),bitmap,myGLContext2,VBO.FIT_XY);
    }

    public synchronized void draw(){
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
