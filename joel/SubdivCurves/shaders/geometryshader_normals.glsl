#version 410
// Geometry shader

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

//calculate normal of line going from p1 to p2
vec2 lineNormal (vec2 p1, vec2 p2) {
    return normalize(vec2(p2.y-p1.y, p1.x-p2.x));
}

vec2 pointNormal (vec2 p1, vec2 p2, vec2 p3) {
    return normalize(lineNormal(p1,p2) + lineNormal(p1,p3));
}


void main() {


    float tau = 6.2831853071;
    int N = 50;

    vec2 pos1;
    vec2 pos2;
    vec2 pos3;



    // hack used in order to order the triangle coordiantes
    // e.g. given ABCDEF the triangles are now ABC BCD DEF
    if (gl_PrimitiveIDIn%2 == 0) {
        pos1 = gl_in[0].gl_Position.xy;
        pos2 = gl_in[1].gl_Position.xy;
        pos3 = gl_in[2].gl_Position.xy;

    } else {
        pos1 = gl_in[1].gl_Position.xy;
        pos2 = gl_in[0].gl_Position.xy;
        pos3 = gl_in[2].gl_Position.xy;
    }

    // angle between two lines
    float angle = acos(dot(normalize(pos3 - pos2), normalize(pos1 - pos2)));

    // curvature from the 3 points
    //float curvature = (2*sin(angle))/length(pos3 - pos1);
    //float curvature = (tau/2.0 - angle);
    float curvature = 2*(tau/2.0 - angle)/(length(pos1-pos2) + length(pos3-pos2));
    //rescale curvature such that it doesn't go off screen
    curvature = sqrt(curvature)/10;


    // normalized normal of vertex
    vec2 pointNormal = normalize((normalize(pos1-pos2) + normalize(pos3-pos2)));

    // position of the vertex
    gl_Position = vec4(pos2,0.0,1.0);
    EmitVertex();

    // normal drawn from the vertex, length changes according to curvature
    gl_Position = vec4(pos2 + pointNormal*curvature,0.0,1.0);
    EmitVertex();

    EndPrimitive();

}
