#version 450 core

layout (location = 0) in vec3 _VertexPosition;

uniform mat4 uMVPMatrix;

void main()
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);
}