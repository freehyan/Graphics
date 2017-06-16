#version 450

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;

out vec4 vs_ProjPos;

void main(void)
{
	vs_ProjPos = vec4(_VertexPosition.xy, 0.9, 1.0);
    gl_Position = vs_ProjPos;
}