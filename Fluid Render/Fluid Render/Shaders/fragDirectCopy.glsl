#version 450 core

uniform sampler2D uColorBuf;

in vec2 g_TexCoord;
out vec4 _FragColor;

void main()
{
	_FragColor = texture(uColorBuf, g_TexCoord);
}