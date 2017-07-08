#version 400 core

layout (location = 0) in vec3 _VertexPosition;
layout (location = 1) in vec2 _VertexTexCoord;
layout (location = 2) in vec3 _VertexNormal; 

uniform mat4 uModelMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uLightSpaceMVPMatrix;

out VS_OUT{
	vec4 LightSpacePos;
	vec2 TexCoord;
	vec3 WorldPos;
	vec3 Normal;
}vs_out;

void main(void)
{
	gl_Position = uMVPMatrix * vec4(_VertexPosition, 1.0);

	vs_out.LightSpacePos = uLightSpaceMVPMatrix * vec4(_VertexPosition, 1.0);
	vs_out.TexCoord = _VertexTexCoord.xy;
	vs_out.Normal = (uModelMatrix * vec4(_VertexNormal, 0.0)).xyz;
	//vs_out.Normal = transpose(inverse(mat3(uModelMatrix))) * _VertexNormal;
	vs_out.WorldPos = (uModelMatrix * vec4(_VertexPosition, 1.0)).xyz;
}