#version 410
// Vertex shader curve

layout (location = 0) in vec2 vertcoords_clip_vs;
layout (location = 1) in float colAttr;

out float color2;

void main() {
  gl_Position = vec4(vertcoords_clip_vs, 0.0, 1.0);
  color2 = colAttr;
}
