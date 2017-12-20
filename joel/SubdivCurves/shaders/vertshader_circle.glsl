#version 410
// Vertex shader

layout (location = 0) in vec2 circle_center_coords;
layout (location = 1) in vec2 circle_point_coords;

out vec4 circlePoint;

void main() {

  gl_Position = vec4(circle_center_coords, 0.0, 1.0);
  circlePoint = vec4(circle_point_coords, 0.0,1.0);

}
