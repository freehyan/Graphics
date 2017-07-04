#version 450 core

in vec3 WorldPos_;
in vec2 TexCoord_;
in vec3 Normal_;

layout (location = 0) out vec4 WorldPosTex_;
layout (location = 1) out vec4 ColorTex_;
layout (location = 2) out vec4 NormalTex_;

uniform sampler2D _uColorTex;

void main()
{	
	WorldPosTex_ = vec4(WorldPos_, 1.0);
	ColorTex_    = texture2D(_uColorTex, TexCoord_);
	NormalTex_   = vec4(normalize(Normal_), 1.0);
}