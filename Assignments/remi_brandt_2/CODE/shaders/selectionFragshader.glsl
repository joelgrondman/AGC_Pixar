#version 410
// Fragment shader selection

layout (location = 0) in vec3 matcolour;

out vec4 fColor;

void main() {
     fColor = vec4(matcolour, 1.0);
}
