precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D yTexture;
uniform sampler2D uvTexture;
void main() {
    vec3 yuv;
    vec3 rgb;
    yuv.r = texture2D(yTexture, vTexCoord).r -16./256;
    yuv.g = texture2D(uvTexture, vTexCoord).r - 0.5;
    yuv.b = texture2D(uvTexture, vTexCoord).a - 0.5;
    rgb = mat3(1.0,       1.0,         1.0,
               0.0,       -0.39465,  2.03211,
               1.13983, -0.58060,  0.0) * yuv;
    gl_FragColor = vec4(rgb, 1.0);
}