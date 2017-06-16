#version 450

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;

out vec2 _TexCoord;

void main(void)
{
	gl_Position = vec4(_VertexPosition.xy, 0.9, 1.0);
	_TexCoord = _VertexTexCoord.xy;
}