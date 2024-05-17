uniform mat4 mvp;
attribute vec4 pos;
attribute vec2 texcoord;
varying vec2 uv;
void main() {
 gl_Position = mvp * pos;
 uv = texcoord;
}
