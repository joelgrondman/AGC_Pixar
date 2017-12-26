#version 410
// Vertex shader selection

layout (location = 0) in vec3 vertcoords_world_vs;
layout (location = 1) in float sharpness;

uniform mat4 modelviewmatrix;
uniform mat4 projectionmatrix;

layout (location = 0) out vec3 matcolor;

void main() {
  gl_Position = projectionmatrix * modelviewmatrix * vec4(vertcoords_world_vs, 1.0);

  if(sharpness == -1.0){
     matcolor =  vec3(0, 1, 0);
  }else if(sharpness == 0){
     matcolor =  vec3(1, 1, 0);
  }else{
     matcolor =  vec3(1, 0, 0);
  }
}
