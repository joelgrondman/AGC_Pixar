#version 410
// Vertex shader

layout (location = 0) in vec3 vertcoords_world_vs;
layout (location = 1) in vec3 vertnormal_world_vs;

uniform mat4 modelviewmatrix;
uniform mat3 normalmatrix;
uniform mat4 projectionmatrix;

//layout (location = 0) out vec3 vertcoords_camera_vs;
//layout (location = 1) out vec3 vertnormal_camera_vs;

out vec3 vertcoords_camera_fs;
out vec3 vertnormal_camera_fs;

void main() {
  //gl_Position = projectionmatrix * modelviewmatrix * vec4(vertcoords_world_vs, 1.0);
  gl_Position = modelviewmatrix * vec4(vertcoords_world_vs, 1.0);

  vertcoords_camera_fs = vec3(modelviewmatrix * vec4(vertcoords_world_vs, 1.0));
  vertnormal_camera_fs = normalmatrix * vertnormal_world_vs;
}
