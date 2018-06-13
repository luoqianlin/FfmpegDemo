attribute vec4 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    vTexCoord=vec2(aTexCoord.x,aTexCoord.y);
    gl_Position = aPosition;
}