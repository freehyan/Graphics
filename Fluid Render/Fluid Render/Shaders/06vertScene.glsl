#version 450 core

layout(location = 0) in vec3 _VertexPosition;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 g_TexCoord;

void main(void)
{
	vec4 Pos = uProjectionMatrix * uViewMatrix * vec4(_VertexPosition, 1.0);
	gl_Position = Pos.xyww;
	g_TexCoord = _VertexPosition;
}