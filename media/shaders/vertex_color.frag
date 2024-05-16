#version 300 es
precision mediump float;
in vec4 vertex_color;
out vec4 fragColor;
void main() {
  fragColor = vertex_color;
}