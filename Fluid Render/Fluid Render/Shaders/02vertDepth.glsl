#version 450 core

layout (location = 0) in vec3 _VertexPosition;

uniform mat4 uWVPMatrix;
uniform mat4 uWorldMatrix;
uniform mat4 uViewMatrix;

uniform float uPointRadius;  // point size in world size
uniform float uPointScale;   // scale to calculate size in pixels

out vec3 g_EyeSpacePos;

void main(void)
{
	vec4 Position = uWVPMatrix *vec4(_VertexPosition.xyz, 1.0);
	gl_Position = Position;

	//gl_PointSize = 10;
	gl_PointSize = 5 * uPointScale * (uPointRadius / Position.w);
	g_EyeSpacePos = (uViewMatrix * uWorldMatrix * vec4(_VertexPosition, 1.0)).xyz;
}