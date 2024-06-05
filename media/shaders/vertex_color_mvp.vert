uniform mat4 mvp;
attribute vec4 pos;
attribute vec3 color;
varying vec3 vertex_color;
void main() {
 gl_Position = mvp * pos;
 vertex_color = color;
}
