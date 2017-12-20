#version 410
// TCS

layout(vertices = 16) out;
in vec3 vPosition[];
out vec3 tcPosition[];
in vec3 vNormal[];
out vec3 tcNormal[];

uniform float tesO1;
uniform float tesO2;
uniform float tesO3;
uniform float tesO4;
uniform float tesi1;
uniform float tesi2;

#define ID gl_InvocationID

void main()
{
    tcPosition[ID] = vPosition[ID];
    tcNormal[ID] = vNormal[ID];

    if (ID == 0) {
        gl_TessLevelInner[0] = tesi1;
        gl_TessLevelInner[1] = tesi2;

        gl_TessLevelOuter[0] = tesO1;
        gl_TessLevelOuter[1] = tesO2;
        gl_TessLevelOuter[2] = tesO3;
        gl_TessLevelOuter[3] = tesO4;
    }
}
