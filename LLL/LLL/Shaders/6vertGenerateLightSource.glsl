#version 450 core

layout (location = 0) in vec3 _VertexPostion;                                          
layout (location = 2) in vec3 _Normal;   

uniform mat4 _uMVPMatrix;
uniform mat4 _uModelMatrix;
uniform mat4 _uNormalMatrix;

out vec3 g_WorldPos;
out vec3 g_Normal;

void main()
{
	gl_Position = _uMVPMatrix * vec4(_VertexPostion, 1.0);
	g_WorldPos = vec3(_uModelMatrix * vec4(_VertexPostion, 1.0)).xyz;
	g_Normal = vec3(_uNormalMatrix * vec4(_Normal, 1.0)).xyz;
}