#version 410
// geometry shader curve

layout(lines_adjacency) in;
layout(line_strip, max_vertices = 8) out;

uniform float showVis; // Indicates which visualization tye is to be shown
uniform float visScale; // Modification variable for visualizations
uniform float totalNumPrimitives; // Total number of primitives in scene

in float color2[]; // Color values received from vertex shader
out vec4 color; // Color value for fragment shader

void main()
{
    if(showVis==0.0){

        // Curvature Visualization: None

        if(gl_PrimitiveIDIn == 0){
            // If is the first processed primitive, draw begin of line
            color = vec4(color2[0],1,color2[0],1);
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();

            color = vec4(color2[1],1,color2[1],1);
            gl_Position = gl_in[1].gl_Position;
            EmitVertex();
        }

        color = vec4(color2[2],1,color2[2],1);
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        color = vec4(color2[3],1,color2[3],1);
        gl_Position = gl_in[3].gl_Position;
        EmitVertex();

    }else if(showVis==1.0){      

        // Curvature Visualization: Curvature combs

        if(gl_PrimitiveIDIn == 0){
            // if is the first processed primitive, draw begin of line
            color = vec4(color2[0],1,color2[0],1);
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();

            color = vec4(color2[1],1,color2[1],1);
            gl_Position = gl_in[1].gl_Position;
            EmitVertex();

            EndPrimitive();
        }

        // Draw curvature comb

        color = vec4(0.0, 0.0, 1.0, 1.0);
        gl_Position = (gl_in[2].gl_Position - ((gl_in[1].gl_Position+gl_in[3].gl_Position)/2))*visScale+gl_in[2].gl_Position;
        EmitVertex();

        color = vec4(0.0, 0.0, 1.0, 1.0);
        gl_Position = (gl_in[1].gl_Position - ((gl_in[0].gl_Position+gl_in[2].gl_Position)/2))*visScale+gl_in[1].gl_Position;
        EmitVertex();

        // Draw curve

        color = vec4(color2[1],1,color2[1],1);
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

        color = vec4(color2[2],1,color2[2],1);
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        if(gl_PrimitiveIDIn == totalNumPrimitives){
            // if is the last processed primitive, draw end of line
            EndPrimitive();

            color = vec4(0.0, 0.0, 1.0, 1.0);
            gl_Position = (gl_in[2].gl_Position - ((gl_in[1].gl_Position+gl_in[3].gl_Position)/2))*visScale+gl_in[2].gl_Position;
            EmitVertex();

            color = vec4(color2[2],1,color2[2],1);
            gl_Position = gl_in[2].gl_Position;
            EmitVertex();

            color = vec4(color2[3],1,color2[3],1);
            gl_Position = gl_in[3].gl_Position;
            EmitVertex();
        }

    }else if(showVis==2.0){

        // Curvature Visualization: Colors

        // Determine color of vertex 1 based on angle with adjacent vertices and offset it by visScale.
        vec2 et = (gl_in[2].gl_Position-gl_in[1].gl_Position).xy;
        vec2 en = (gl_in[0].gl_Position-gl_in[1].gl_Position).xy;
        float c = (dot(normalize(en),normalize(et))+1)/2.0;

        if(gl_PrimitiveIDIn == 0){
            // If is the first processed primative, draw begin of line
            color = vec4(-.5+c*2+visScale/100,1-c*2-visScale/100,0,1);
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();
        }

        // Draw vertex 1
        color = vec4(-.5+c*2+visScale/100,1-c*2-visScale/100,0,1);
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

        // Determine color of vertex 2 based on angle with adjacent vertices and offset it by visScale.
        et = (gl_in[3].gl_Position-gl_in[2].gl_Position).xy;
        en = (gl_in[1].gl_Position-gl_in[2].gl_Position).xy;
        c = (dot(normalize(en),normalize(et))+1)/2.0;
        color = vec4(-.5+c*2+visScale/100,1-c*2-visScale/100,0,1);
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        if(gl_PrimitiveIDIn == totalNumPrimitives){
            // If is the last processed primative, draw end of line
            color = vec4(-.5+c*2+visScale/100,1-c*2-visScale/100,0,1);
            gl_Position = gl_in[3].gl_Position;
            EmitVertex();
        }
    }else if(showVis==3.0){

        // Curvature Visualization: Centers of Osculating Circles

        // Determine position of center of osculating circle
        vec2 one = gl_in[0].gl_Position.xy;
        vec2 two = gl_in[1].gl_Position.xy;
        vec2 three = gl_in[2].gl_Position.xy;

        float ma = (two.y-one.y)/(two.x-one.x); // slope line through one and two
        float mb = (three.y-two.y)/(three.x-two.x); // slope line through three and two
        float x = (ma*mb*(one.y-three.y)+mb*(one.x+two.x)-ma*(two.x+three.x))/(2*(mb-ma)); // x posiition center
        float y = (-1/ma)*(x - (one.x+two.x)/2)+(one.y+two.y)/2; // y position center
        // float r = sqrt((one.x-x)*(one.x-x)+(one.y-y)*(one.y-y)); // radius circle (added for extendability)

        // Draw center
        color = vec4(0,0,1,1);
        gl_Position = vec4(x-.001,y-.001,0,1);
        EmitVertex();
        gl_Position = vec4(x+.001,y+.001,0,1);
        EmitVertex();
        gl_Position = vec4(x-.001,y+.001,0,1);
        EmitVertex();
        gl_Position = vec4(x+.001,y-.001,0,1);
        EmitVertex();

        EndPrimitive();

        if(gl_PrimitiveIDIn == 0){
            // If is the first processed primative, draw begin of line
            color = vec4(color2[0],1,color2[0],1);
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();

            color = vec4(color2[1],1,color2[1],1);
            gl_Position = gl_in[1].gl_Position;
            EmitVertex();
        }

        // Draw curve

        color = vec4(color2[2],1,color2[2],1);
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        color = vec4(color2[3],1,color2[3],1);
        gl_Position = gl_in[3].gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}
