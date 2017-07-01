#version 450 core

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoords;
layout (location = 2) in vec3 _VertexNormal;

uniform mat4 uMVPMatrix;
uniform mat4 uNormalMatrix;

out vec2 TexCoords;
out vec3 _NormalE;

void main()
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);
	_NormalE = vec3(uNormalMatrix * vec4(_VertexNormal, 1.0)).xyz;;
	TexCoords = _VertexTexCoords;
}