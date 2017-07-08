#version 400 core

in vec2 g_TexCoord;

uniform sampler2D uShadowMapTexture;

layout (location = 0) out vec4 FragColor_;

void main()
{
	vec4 SampledColor = texture(uShadowMapTexture, g_TexCoord);
	FragColor_ = SampledColor;

	//float Depth = texture(uShadowMapTexture, g_TexCoord).r;

	//float NDCZ = Depth * 2 - 1;
	//float Near = 0.1;
	//float Far = 100.0;

	//float LinearDepth = (2*Near) / (Far+Near - NDCZ*(Far-Near));
	//FragColor_ = vec4(LinearDepth, LinearDepth, LinearDepth, 1.0);
}