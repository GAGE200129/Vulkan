#version 450


layout(location = 0) in float FSHeight;


layout(location = 0) out vec4 outColor;

const vec4 gLowColor = vec4(0.6, 0.38, 0.66, 1.0);
const vec4 gHighcolor = vec4(0.0, 0.15, 0.66, 1.0);
const vec4 gBaseColor = vec4(0.18, 0.27, 0.47, 1.0);


void main() { 
    float red = -0.22 * FSHeight + gBaseColor.x; 
    float green = -0.25 * FSHeight + gBaseColor.y; 
    float blue = -0.19 * FSHeight + gBaseColor.z; 

    

    outColor = vec4(red, green, blue, 1);
}   