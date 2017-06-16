#version 450

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;
layout (location = 2) in vec3 _VertexNormal;

out vec4 vs_ViewPos;
out vec3 vs_WorldNormal;
out vec2 vs_TexCoord;

uniform mat4 uModelViewMatrix;
uniform mat4 uModelViewProjMatrix;
uniform mat4 uNormalMatrix;

void main(void)
{
    vs_ViewPos = uModelViewMatrix * vec4(_VertexPosition, 1.0);
    vs_WorldNormal = _VertexNormal;
    vs_TexCoord = _VertexTexCoord;
 
    gl_Position = uModelViewProjMatrix * vec4(_VertexPosition, 1.0);
}
