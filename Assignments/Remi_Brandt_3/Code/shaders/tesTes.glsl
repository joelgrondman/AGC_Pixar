#version 410
// TES

layout(quads) in;

layout (location = 0) out vec3 vertcoords_camera_fs;
layout (location = 1) out vec3 vertnormal_camera_fs;

in vec3 tcPosition[];
in vec3 tcNormal[];

out vec3 tePosition;
out vec4 tePatchDistance;

uniform mat4 modelviewmatrix;
uniform mat4 projectionmatrix;
uniform mat3 normalmatrix;

float fact(int num){
    float res = 1;
    for(int i=1; i<=num; i++){
        res *= i;
    }
    return res;
}

vec3 calcPos(float u, float v){
    // Determine position on bezier patch.
    vec3 pos = vec3(0.0,0.0,0.0);
    for(int j = 0; j < 4; j++){
        for(int i = 0; i < 4; i++){
            pos = pos + ((fact(3)/(fact(i)*fact(3-i)) * pow(u,i) * pow(1-u, 3-i))
                      * (fact(3)/(fact(j)*fact(3-j)) * pow(v,j) * pow(1-v, 3-j))
                      * tcPosition[i + j * 4]);
        }
    }
    return pos;
}


vec3 calcNormal(float u, float v){
    // Determine normal of bezier patch.
    vec3 normal = vec3(0.0,0.0,0.0);
    for(int j = 0; j < 4; j++){
        for(int i = 0; i < 4; i++){
            normal = normal + ((fact(3)/(fact(i)*fact(3-i)) * pow(u,i) * pow(1-u, 3-i))
                      * (fact(3)/(fact(j)*fact(3-j)) * pow(v,j) * pow(1-v, 3-j))
                      * tcNormal[i + j * 4]);
        }
    }
    return normal;
}

#define ID gl_InvocationID

void main()
{
   float u = gl_TessCoord.x, v = gl_TessCoord.y;

    // Move all vertices somewhere on center quad (center quad of the 3x3 quads).
    u = u/(3.0) + 1/(3.0);
    v = v/(3.0) + 1/(3.0);

    vec3 pos =  calcPos(u, v); // Determine position of vertex in model coordinates.
    vec3 normal = calcNormal(u,v); // Determine normal of vertex in model coordinates.

    vertcoords_camera_fs = vec3(modelviewmatrix * vec4(pos, 1.0));
    vertnormal_camera_fs = normalize(normalmatrix  * normal);

    tePosition =  pos;
    tePatchDistance = vec4(u, v, 1-u, 1-v);
    gl_Position = projectionmatrix * modelviewmatrix * vec4(tePosition, 1);
}
