#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec2 outUV;

struct Vertex 
{
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer
{ 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{	
	VertexBuffer vertexBuffer;
} PushConstants;




void main() 
{
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output the position of each vertex
	gl_Position = vec4(v.position, 1.0f);
}