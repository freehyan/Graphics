#version 450

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;

void main(void)
{
    gl_Position = vec4(_VertexPosition.xy, 0.9, 1.0);
}