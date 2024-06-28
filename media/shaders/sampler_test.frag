precision highp float;
varying vec2 out_texcoord;
uniform sampler2D texSampler;
void main() {
  vec4 samplerColor = texture2D(texSampler, out_texcoord);
  gl_FragColor = vec4( samplerColor.r, samplerColor.g, samplerColor.b, samplerColor.a );
}