#version 450 core

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;                                             
layout (location = 2) in vec3 _VertexNormal;   

uniform mat4 uMVPMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

out	vec2 g_TexCoord;
out vec3 g_Normal;
out vec3 g_WorldPos;

void main()
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);
	g_TexCoord = _VertexTexCoord;
	g_WorldPos = vec3(uModelMatrix * vec4(_VertexPosition, 1.0)).xyz;
	g_Normal = vec3(uNormalMatrix * vec4(_VertexNormal, 1.0)).xyz;
}