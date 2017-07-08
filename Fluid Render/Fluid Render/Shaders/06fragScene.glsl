#version 450

layout(location = 0) out vec4 _SkyBoxColor;

in vec3 g_TexCoord;
uniform samplerCube uSkyBoxTex;

void main(void)
{
	_SkyBoxColor = texture(uSkyBoxTex, g_TexCoord);
}