package gl.module;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.opengles.GL10;

import gl.context.MyGLContext;

public class MyTriangle {
    private MyGLContext glContext;
    private   float[] vertex ;
    private float deltaX=0.1f;

    private  FloatBuffer mVertexBuffer;

    float color[] = { 255,255, 0, 1.0f };

    public MyTriangle(MyGLContext glContext,float[] vertex) {
        this.vertex=vertex;
        mVertexBuffer = ByteBuffer.allocateDirect(vertex.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertex);
        mVertexBuffer.flip();
        this.glContext=glContext;

    }

    public float getDeltaX() {
        return deltaX;
    }

    public void setDeltaX(float deltaX) {
        this.deltaX = deltaX;
    }

    public float[] getColor() {
        return color;
    }

    public void setColor(float[] color) {
        this.color = color;
    }

    public void draw(GL10 gl){
        glContext.useProgram();
        GLES20.glEnableVertexAttribArray(glContext.getPositionHandle());
        GLES20.glVertexAttribPointer(glContext.getPositionHandle(), 3, GLES20.GL_FLOAT, false,
                12, mVertexBuffer);
        // Set color for drawing the triangle
        GLES20.glUniform4fv(glContext.getColorHandle(), 1, color, 0);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 3);
        GLES20.glDisableVertexAttribArray(glContext.getPositionHandle());
    }


    public  float[] getVertex() {
        return vertex;
    }

    public  void setVertex(float[] vertex) {
        this.vertex=vertex;
        mVertexBuffer = ByteBuffer.allocateDirect(vertex.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertex);
        mVertexBuffer.flip();
    }
}
