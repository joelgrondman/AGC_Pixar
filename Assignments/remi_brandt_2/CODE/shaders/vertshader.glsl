#version 410
// Vertex shader

layout (location = 0) in vec3 vertcoords_world_vs;
layout (location = 1) in vec3 vertnormal_world_vs;

uniform mat4 modelviewmatrix;
uniform mat4 projectionmatrix;
uniform mat3 normalmatrix;

layout (location = 0) out vec3 vertcoords_camera_vs;
layout (location = 1) out vec3 vertnormal_camera_vs;
layout (location = 2) out vec3 matcolor;


void main() {
  gl_Position = projectionmatrix * modelviewmatrix * vec4(vertcoords_world_vs, 1.0);

  vertcoords_camera_vs = vec3(modelviewmatrix * vec4(vertcoords_world_vs, 1.0));
  vertnormal_camera_vs = normalize(normalmatrix * vertnormal_world_vs);

  matcolor =  vec3(0.53, 0.80, 0.87);
}
