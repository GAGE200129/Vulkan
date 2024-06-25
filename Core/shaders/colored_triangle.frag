#version 450

layout (location = 0) in vec2 outUV;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
	//return red
	outFragColor = vec4(0.0f, 1.0f, 0.0f ,1.0f);
}