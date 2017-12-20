#version 410
// Fragment shader

in vec4 fColorCurvature;

out vec4 fColor;

void main() {
  // either cyan if geometry shader is used and otherwise blue
  fColor = fColorCurvature;

}
