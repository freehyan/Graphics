#version 450

in vec3 NormalW;
in vec2 TexCoords;
in vec3 WorldPosW;

uniform sampler2D uColorTex;

layout(location = 0) out vec4 PositionTex;
layout(location = 1) out vec4 NormalTex;
layout(location = 2) out vec4 DiffuseColorTex;

void main()
{	
	PositionTex = vec4(WorldPosW, 1.0);
	NormalTex = vec4(normalize(NormalW), 1.0);
	DiffuseColorTex = texture2D(uColorTex, TexCoords);
}