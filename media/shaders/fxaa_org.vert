uniform mat4 mvp;
attribute vec4 pos;
attribute vec2 texcoord;
varying vec2 v_texCoords;
void main() {
 gl_Position = mvp * pos;
 v_texCoords = texcoord;
}