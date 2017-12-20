#version 410
// Vertex shader

layout (location = 0) in vec3 vertcoords_world_vs;
layout (location = 1) in vec3 vertnormal_world_vs;

out vec3 vPosition;
out vec3 vNormal;

void main() {
  vPosition = vertcoords_world_vs;
  vNormal = vertnormal_world_vs;
}
