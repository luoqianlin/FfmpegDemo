package gl.module;

import android.graphics.PointF;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;

/**
 * @author lql E-mail: 595308107@qq.com
 * @version 0 创建时间：2018/6/13 12:39
 * 类说明
 */
public class VBO {
    public static final int FIT_XY = 0;
    public static final int ASPECT_RATIO = 1;
    public static final int FIT_REPEAT = 2;
    public static final int CUT_GRAVITY_TOP_RIGHT = 3;
    public static final int CUT_GRAVITY_TOP_LEFT = 4;

    protected   final float[] VERTEX = {   // in counterclockwise order:
            1f, 0.5f, 0.0f,   // top right
            -1.0f, 0.5f, 0.0f,  // top left
            -1.0f, -0.5f, 0.0f, // bottom left
            1.0f, -0.5f, 0.0f,  // bottom right
    };
    protected  final short[] VERTEX_INDEX = {
            0, 1, 2, 0, 2, 3
    };
    protected  final float[] TEX_VERTEX = {   // in clockwise order:
            1f, 0,  // bottom right
            0f, 0,  // bottom left
            0f, 1f,  // top left
            1f, 1,  // top right
    };

    protected final FloatBuffer mVertexBuffer;
    protected final FloatBuffer mTexVertexBuffer;
    protected final ShortBuffer mVertexIndexBuffer;

    protected int x;
    protected int y;

    protected int winWidth;
    protected int winHeight;

    protected int targetWidth;
    protected int targetHeight;

    protected int displyType= VBO.FIT_XY;

    protected int rawWidth;
    protected int rawHeight;

    public VBO(int x, int y,
               int winWidth, int winHeight,
               int targetWidth, int targetHeight) {
        System.out.println("构造函数");
        this.winHeight=winHeight;
        this.winWidth=winWidth;
        this.targetWidth = targetWidth;
        this.targetHeight = targetHeight;
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

        mTexVertexBuffer = ByteBuffer.allocateDirect(TEX_VERTEX.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(TEX_VERTEX);
        mTexVertexBuffer.flip();

        mVertexIndexBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2)
                .order(ByteOrder.nativeOrder())
                .asShortBuffer()
                .put(VERTEX_INDEX);
        mVertexIndexBuffer.flip();

    }


    public void setDisplyType(int rawWidth, int rawHeight, int displyType){
        setDisplyType(this.x,rawWidth,rawHeight,displyType);
    }

    public void setDisplyType(int x,int rawWidth, int rawHeight, int displyType) {
        if (this.x== x && rawWidth == this.rawWidth && rawHeight == this.rawHeight
                && displyType == this.displyType) {
            return;
        }
        this.x = x;
        this.rawWidth = rawWidth;
        this.rawHeight = rawHeight;
        this.displyType = displyType;
        invalidate();
    }

    public void invalidate() {
        int targetWidth = this.targetWidth;
        int targetHeight = this.targetHeight;
        if(displyType== VBO.FIT_REPEAT){
            float w_factor = targetWidth * 1.0f / rawWidth;
            float h_factor = targetHeight * 1.0f / rawHeight;
            for (int k = 0; k < TEX_VERTEX.length; k += 2) {
                if (k == 0) {
                    TEX_VERTEX[k] = w_factor;
                } else if (k == 4) {
                    TEX_VERTEX[k + 1] = h_factor;
                } else if (k == 6) {
                    TEX_VERTEX[k] = w_factor;
                    TEX_VERTEX[k + 1] = h_factor;
                }
            }
        }else if(displyType == VBO.ASPECT_RATIO){
            float wh_ratio =rawWidth * 1.0f / rawHeight;
            float twh_ratio = targetWidth * 1.0f / targetHeight;
            if (wh_ratio > twh_ratio) {
                targetHeight = (int) (targetWidth / wh_ratio + 0.5);
            } else {
                targetWidth = (int) (targetHeight * wh_ratio + 0.5);
            }
        }else if(displyType== CUT_GRAVITY_TOP_RIGHT){
            float w_factor = targetWidth * 1.0f / rawWidth;
            float h_factor = targetHeight * 1.0f / rawHeight;
            if (w_factor < 1.0f) {
                TEX_VERTEX[2] = 1 - w_factor;
                TEX_VERTEX[4] = 1 - w_factor;
            } else {
                targetWidth = rawWidth;
            }
            if (h_factor < 1.0f) {
                TEX_VERTEX[5] = h_factor;
                TEX_VERTEX[7] = h_factor;
            } else {
                targetHeight = rawHeight;
            }
        }else if(displyType == CUT_GRAVITY_TOP_LEFT){
            float w_factor = targetWidth * 1.0f / rawWidth;
            float h_factor = targetHeight * 1.0f / rawHeight;
            if (w_factor < 1.0f) {
                TEX_VERTEX[0] = w_factor;
                TEX_VERTEX[6] = w_factor;
            } else {
                targetWidth = rawWidth;
            }
            if (h_factor < 1.0f) {
                TEX_VERTEX[5] = h_factor;
                TEX_VERTEX[7] = h_factor;
            } else {
                targetHeight = rawHeight;
            }
        }
        System.out.println("TEX_VERTEX=>"+ Arrays.toString(TEX_VERTEX));

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

        System.out.println("****>"+ Arrays.toString(VERTEX));
        rebuildVBO();
    }

    float deltaTex = 0.01f;
    boolean initdeltaTexx=false;

    public void moveTex(){
        if (!initdeltaTexx) {
            deltaTex = -5f / this.rawWidth * 1.0f;
            initdeltaTexx = true;
        }
        System.out.println("deltaTex:"+deltaTex);
        if (TEX_VERTEX[0] >= 1.0f && deltaTex > 0) {
            deltaTex = -1 * deltaTex;
        } else if (TEX_VERTEX[2] <= 0 && deltaTex < 0) {
            deltaTex = -1 * deltaTex;
        }
        for (int i = 0; i < TEX_VERTEX.length; i += 2) {
            TEX_VERTEX[i] += deltaTex;
        }
        System.out.println("TEX_VERTEX >> "+Arrays.toString(TEX_VERTEX));
        mTexVertexBuffer.clear();
        mTexVertexBuffer.put(TEX_VERTEX);
        mTexVertexBuffer.flip();
    }

    private void rebuildVBO() {
        mVertexBuffer.clear();
        mVertexBuffer.put(VERTEX);
        mVertexBuffer.flip();

        mTexVertexBuffer.clear();
        mTexVertexBuffer.put(TEX_VERTEX);
        mTexVertexBuffer.flip();
    }

    public void toGLCoordinate(int x, int y, int winWidth, int winHeight,PointF pointF) {
        pointF.x=2.0f * x / winWidth - 1.0f;
        pointF.y=1.0f - (2.0f * y / winHeight);
    }

}
