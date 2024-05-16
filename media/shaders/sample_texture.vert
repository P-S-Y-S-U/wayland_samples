uniform mat4 mvp;
attribute vec4 pos;
attribute vec2 texcoord;
varying vec2 out_texcoord;
void main() {
 gl_Position = mvp * pos;
 out_texcoord = texcoord;
}
