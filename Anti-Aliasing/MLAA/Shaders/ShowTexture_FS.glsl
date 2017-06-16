#version 450

in vec2 _TexCoord;

uniform sampler2D uColorTex;
uniform int uShowMeshId;
layout(location = 0) out vec4 FS_FinalColor;

void main()
{
	vec2 TexCoord = vec2(0.0);
	if (uShowMeshId == 0)
		TexCoord = vec2(_TexCoord.x, _TexCoord.y);
	else
		TexCoord = vec2(_TexCoord.x, 1.0 - _TexCoord.y);
	FS_FinalColor = texture2D(uColorTex, TexCoord);
}