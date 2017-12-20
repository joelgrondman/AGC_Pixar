#version 410
// Geometry shader

layout (triangles) in;
layout (line_strip, max_vertices = 3) out;

out vec4 fColorCurvature;

//calculate normal of line going from p1 to p2
vec2 lineNormal (vec2 p1, vec2 p2) {
    return normalize(vec2(p2.y-p1.y, p1.x-p2.x));
}

vec2 pointNormal (vec2 p1, vec2 p2, vec2 p3) {
    return normalize(lineNormal(p1,p2) + lineNormal(p1,p3));
}

void main() {

    float tau = 6.2831853071;

    vec2 pos1;
    vec2 pos2;
    vec2 pos3;

    // hack used to reorder the triangle coordiantes
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

    //rescale curvature to a range that causes less overlap
    curvature /=5;

    // draw the line polygon where color scales with curvature
    // note the different alpha values, since the different lines generated
    // by the geometry shader have no knowledge about one another we can't
    // interpolate their color directly, instead we change the alpha value
    // and let the opengl blend the different lines together
    // creating the illusion of continuity

    // clamp color and add a constant value to it such that all parts can be seen
    float colorFactor = (clamp(curvature*0.7,0.0,1.0)*0.7 + 0.3);

    gl_Position = vec4(pos1,0.0,1.0);
    fColorCurvature = colorFactor*vec4(0.0,1.0,1.0,0.0);

    EmitVertex();

    gl_Position = vec4(pos2,0.0,1.0);
    fColorCurvature = colorFactor*vec4(0.0,1.0,1.0,1.0);

    EmitVertex();


    gl_Position = vec4(pos3,0.0,1.0);
    fColorCurvature = colorFactor*vec4(0.0,1.0,1.0,0.0);

    EmitVertex();

    EndPrimitive();


}
