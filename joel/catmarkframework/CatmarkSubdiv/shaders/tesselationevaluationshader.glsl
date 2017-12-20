#version 410
// tesslation shader

layout(quads) in;

uniform mat4 modelviewmatrix;
uniform mat3 normalmatrix;
uniform mat4 projectionmatrix;

in vec3 vertcoords_camera_cs[];
in vec3 vertnormal_camera_cs[];

out vec3 vertcoords_camera_fs;
out vec3 vertnormal_camera_fs;

void main(void){
    float u = gl_TessCoord.x, v = gl_TessCoord.y;

    //outer vertices are pushed towards inner vertices in order
    //to prevent overlapping patches
  //  u = clamp(u,1.0/3.0,2.0/3.0);
    //v = clamp(v,1.0/3.0,2.0/3.0);
    u = (u/3.0 + 1.0/3.0);
    v = (v/3.0 + 1.0/3.0);

    //adjust vertex coordinates
    vec3 vc[] = vertcoords_camera_cs;
    mat4 Px = mat4(
                vc[0].x, vc[1].x, vc[2].x, vc[3].x,
                vc[4].x, vc[5].x, vc[6].x, vc[7].x,
                vc[8].x, vc[9].x, vc[10].x, vc[11].x,
                vc[12].x, vc[13].x, vc[14].x, vc[15].x );

    mat4 Py = mat4(
        vc[0].y, vc[1].y, vc[2].y, vc[3].y,
        vc[4].y, vc[5].y, vc[6].y, vc[7].y,
        vc[8].y, vc[9].y, vc[10].y, vc[11].y,
        vc[12].y, vc[13].y, vc[14].y, vc[15].y );

    mat4 Pz = mat4(
        vc[0].z, vc[1].z, vc[2].z, vc[3].z,
        vc[4].z, vc[5].z, vc[6].z, vc[7].z,
        vc[8].z, vc[9].z, vc[10].z, vc[11].z,
        vc[12].z, vc[13].z, vc[14].z, vc[15].z );

    const mat4 B = mat4(-1.0,3.0,-3.0,1.0,
                  3.0,-6.0,3.0,0.0,
                  -3.0,3.0,0.0,0.0,
                  1.0,0.0,0.0,0.0);
    const mat4 BT = transpose(B);


    mat4 cx = B * Px * BT;
    mat4 cy = B * Py * BT;
    mat4 cz = B * Pz * BT;

    vec4 U = vec4(u*u*u, u*u, u, 1);
    vec4 V = vec4(v*v*v, v*v, v, 1);

    float x = dot(cx * V, U);
    float y = dot(cy * V, U);
    float z = dot(cz * V, U);

    gl_Position = projectionmatrix * vec4(x, y, z, 1);
    vertcoords_camera_fs = vec3(x, y, z);

    //adjust normal coordinates
    vc = vertnormal_camera_cs;
    Px = mat4(
                vc[0].x, vc[1].x, vc[2].x, vc[3].x,
                vc[4].x, vc[5].x, vc[6].x, vc[7].x,
                vc[8].x, vc[9].x, vc[10].x, vc[11].x,
                vc[12].x, vc[13].x, vc[14].x, vc[15].x );

    Py = mat4(
        vc[0].y, vc[1].y, vc[2].y, vc[3].y,
        vc[4].y, vc[5].y, vc[6].y, vc[7].y,
        vc[8].y, vc[9].y, vc[10].y, vc[11].y,
        vc[12].y, vc[13].y, vc[14].y, vc[15].y );

    Pz = mat4(
        vc[0].z, vc[1].z, vc[2].z, vc[3].z,
        vc[4].z, vc[5].z, vc[6].z, vc[7].z,
        vc[8].z, vc[9].z, vc[10].z, vc[11].z,
        vc[12].z, vc[13].z, vc[14].z, vc[15].z );

    cx = B * Px * BT;

    cy = B * Py * BT;

    cz = B * Pz * BT;


    x = dot(cx * V, U);
    y = dot(cy * V, U);
    z = dot(cz * V, U);

    vertnormal_camera_fs = vec3(x, y, z);


}
