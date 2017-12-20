#version 410
// Vertex shader control net

layout (location = 0) in vec2 vertcoords_clip_vs;

out vec4 color;

void main() {
  gl_Position = vec4(vertcoords_clip_vs, 0.0, 1.0);
  color = vec4(1, 0, 0, 1.0);
}
