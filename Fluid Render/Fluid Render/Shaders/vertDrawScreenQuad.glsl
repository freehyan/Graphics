#version 400 core

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;

out vec2 g_TexCoord;

void main(void)
{
	gl_Position = vec4(_VertexPosition.xy, 0.0, 1.0);
	g_TexCoord = _VertexTexCoord.xy;
}