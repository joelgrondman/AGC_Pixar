#version 410
// tesselation shader

layout (vertices = 16) out;

uniform float inner1;
uniform float inner2;
uniform float outer;


in vec3 vertcoords_camera_fs[];
in vec3 vertnormal_camera_fs[];

out vec3 vertcoords_camera_cs[];
out vec3 vertnormal_camera_cs[];

void main(void){


    if (gl_InvocationID == 0){
        gl_TessLevelInner[0] = inner1;
        gl_TessLevelInner[1] = inner2;
        gl_TessLevelOuter[0] = outer;
        gl_TessLevelOuter[1] = outer;
        gl_TessLevelOuter[2] = outer;
        gl_TessLevelOuter[3] = outer;

//in case of quad, you have to specify both gl_TessLevelInner[1] and //gl_TessLevelOuter[3]
    }
    //gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    vertcoords_camera_cs[gl_InvocationID] = vertcoords_camera_fs[gl_InvocationID];
    vertnormal_camera_cs[gl_InvocationID] = vertnormal_camera_fs[gl_InvocationID];

}
