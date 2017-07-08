#version 450 core

layout (location = 0) in vec3 _VertexPosition;

uniform mat4 uMVPMatrix;

out float g_DepthZ;
//out float g_DepthW;

void main()
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);
	g_DepthZ = gl_Position.z / gl_Position.w;
	//g_DepthZ = gl_Position.z;
	//g_DepthW = gl_Position.w;
}