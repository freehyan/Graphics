#version 450 core

in vec2 g_TexCoord;

uniform sampler2D uShadowMapTexture;
uniform float uAlpha;
uniform int uTextureWidth;
uniform int uTextureHeight;

layout (location = 0) out vec4 FragColor_;

const int KernelSize = 3;

//***********************************************************
//FUNCTION:: 3*3 box filter
float boxFilter(in vec2 vTextureCoord)
{
	float TextureOffsetX = 1.0 / uTextureWidth;
	float TextureOffsetY = 1.0 / uTextureHeight;

	float SumPixelDepthZ = 0.0;
	int HalfKernelSize = KernelSize / 2;
	for (int i=-HalfKernelSize; i<=HalfKernelSize; i++)
	{
		for (int k=-HalfKernelSize; k<=HalfKernelSize; k++)
		{
			vec2 TextureCoord = vTextureCoord + vec2(TextureOffsetX*i, TextureOffsetY*k);
			SumPixelDepthZ += texture2D(uShadowMapTexture, TextureCoord).r; 
		}
	}

	float ExponentialValue = SumPixelDepthZ / 9.0;
	return ExponentialValue;
}

void main()
{
	float FilterDepthZ = boxFilter(g_TexCoord);

	FragColor_ = vec4(FilterDepthZ, FilterDepthZ, FilterDepthZ, 1.0);
}