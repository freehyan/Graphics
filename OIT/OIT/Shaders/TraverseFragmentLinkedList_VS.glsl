#version 450 core

layout (location = 0) in vec3 _VertexPosition;

void main()
{
	gl_Position = vec4(_VertexPosition.xy, 0.0, 1.0);
}