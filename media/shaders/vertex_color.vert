#version 300 es
out vec4 vertex_color;
const vec2 positions[3] = vec2[3](
vec2(0.0, -0.5),
vec2(0.5, 0.5),
vec2(-0.5, 0.5)
);
const vec3 colors[3] = vec3[3](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);
void main() {
 gl_Position = vec4( positions[gl_VertexID], 0.0, 1.0 );
 vertex_color = vec4( colors[gl_VertexID], 1.0 );
}