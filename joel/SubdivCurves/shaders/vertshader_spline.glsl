#version 410
// Vertex shader

layout (location = 0) in vec2 vertcoords_clip_vs;

out vec4 fColorCurvature;

void main() {

  gl_Position = vec4(vertcoords_clip_vs, 0.0, 1.0);
  fColorCurvature = vec4(0.0,0.0,1.0,1.0);

}
