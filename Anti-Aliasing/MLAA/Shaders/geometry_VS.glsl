#version 450

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoords;
layout (location = 2) in vec3 _VertexNormal;

uniform mat4 uMVPMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

out vec2 TexCoords;
out vec3 NormalW;
out vec3 WorldPosW;

void main()
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);
	NormalW = vec3(uNormalMatrix * vec4(_VertexNormal, 1.0)).xyz;
	WorldPosW = vec3(uModelMatrix * vec4(_VertexPosition, 1.0)).xyz;
	TexCoords = _VertexTexCoords;
}