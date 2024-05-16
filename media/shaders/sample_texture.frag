precision highp float;
varying vec2 out_texcoord;
uniform sampler2D texSampler;
void main() {
  gl_FragColor = texture2D(texSampler, out_texcoord);
}