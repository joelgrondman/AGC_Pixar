#version 410
// Geometry shader

layout (triangles) in;
layout (line_strip, max_vertices = 256) out;

uniform int circlePos;
uniform bool allCircles;

//calculate normal of line going from p1 to p2
vec2 lineNormal (vec2 p1, vec2 p2) {
    return normalize(vec2(p2.y-p1.y, p1.x-p2.x));
}

void main() {

    // Only draw one circle given by circlePos or draw all circles if allCircles is 1
    if (gl_PrimitiveIDIn != circlePos && !allCircles)
        return;

    float tau = 6.2831853071;
    // number of vertices used to draw the circle
    int N = 256;

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
    //curvature = sqrt(curvature);
    //curvature /=5;

    // normalized normal of vertex
    vec2 pointNormal = normalize((normalize(pos1-pos2) + normalize(pos3-pos2)));

    if(curvature < 0.001) {     //case in which curvature is 0, a line is drawn tangent to the spline and offset in normal direction

        vec2 tangent = normalize((pos2-pos1) + (pos3-pos2));

        gl_Position = vec4(pos2 + tangent,0.0,1.0);
        EmitVertex();

        gl_Position = vec4(pos2 - tangent,0.0,1.0);
        EmitVertex();
    } else {     //drawing circles

        float radius = 1.0/curvature;

        for (int i = 0; i < N; ++i) {

            gl_Position = vec4(pos2 + pointNormal*radius + radius*vec2(cos(tau * float(i)/(N-1)),sin(tau * float(i)/(N-1)))
                               ,0.0,1.0);
            EmitVertex();

        }
    }

    EndPrimitive();

}
