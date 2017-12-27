#version 410
// Vertex shader selection

layout (location = 0) in vec3 vertcoords_world_vs;
layout (location = 1) in float sharpness;

uniform mat4 modelviewmatrix;
uniform mat4 projectionmatrix;
uniform int displayCreases;

layout (location = 0) out vec3 matcolor;

vec3 rainbowmap(float value){
    if (value < 256)    {
        return vec3(1, value/255, 0);
    }    else if (value < 512)    {
        value -= 256;
        return vec3((255 - value)/255, 1, 0);
    }else if (value < 768)    {
        value -= 512;
        return vec3(0, 1, value);
    }else{
        value -= 768;
        return vec3(0, (255 - value)/255, 1);
    }
}

void main() {
    gl_Position = projectionmatrix * modelviewmatrix * vec4(vertcoords_world_vs, 1.0);

    if(displayCreases==1){
        matcolor =  rainbowmap(sharpness);
    }else{
        if(sharpness == -1.0){
           matcolor =  vec3(0, 1, 0);
        }else if(sharpness == 0){
           matcolor =  vec3(1, 1, 0);
        }else{
           matcolor =  vec3(1, 0, 0);
        }
    }
}
