#version 450

layout (location = 0) out vec4 _ShadedNormal;
layout (location = 1) out vec4 _FluidColor;

in vec2 g_TexCoord;

uniform sampler2D uFluidDepthTex;
uniform sampler2D uBilateralNormalTex;
uniform vec2 uScreenSize;
uniform int uSampleOffset;

uniform vec3 uLightDir = vec3(1.0, 1.0, 1.0);
uniform vec3 uFluidColor = vec3(0.1, 0.2, 0.5);

//--------------------------------------------------------------------------------------
vec3 transformNormalBack(in vec3 f3Normal)
{
	vec3 f3RetNormal = (f3Normal - vec3(0.5)) * 2.0f;
	return f3RetNormal;
}

//--------------------------------------------------------------------------------------
// main function
//--------------------------------------------------------------------------------------
void main()
{
	vec2 f2ViewSpaceCoord = gl_FragCoord.xy;
	vec2 f2TexCoord       = f2ViewSpaceCoord / uScreenSize;
	float x				  = gl_FragCoord.x;
	float y				  = gl_FragCoord.y;

// read eye-space depth from texture
	float fFluidDepth = texture(uFluidDepthTex, g_TexCoord).r;
	vec3 f3Normal = vec3(0.0, 0.0, 0.0);

	if ((int(x) % uSampleOffset == 0) && (int(y) % uSampleOffset == 0))
	{	
		f3Normal = texture(uBilateralNormalTex, f2TexCoord).xyz;
	}
	else
	{
		if (fFluidDepth > 0.0)
		{
			int iGridX = int(floor(x / uSampleOffset) * uSampleOffset);
			int iGridY = int(floor(y / uSampleOffset) * uSampleOffset);

			float fDistPercentX = (x - iGridX) * 1.0f / uSampleOffset;
			float fDistPercentY = (y - iGridY) * 1.0f / uSampleOffset;

			vec3 f3SampleNormal[4];
			for (uint i=0; i<4; i++)
				f3SampleNormal[i] = vec3(0.0);

			vec2 f2TexCoord0  = vec2(iGridX, iGridY) / uScreenSize;
			vec3 f3TempNormal = texture(uBilateralNormalTex, f2TexCoord0).xyz;
			f3SampleNormal[0]   = transformNormalBack(f3TempNormal);

			if ((x+1 < uScreenSize.x) && (iGridX+uSampleOffset < uScreenSize.x))
			{
				vec2 f2TexCoord1 = vec2(iGridX+uSampleOffset, iGridY) / uScreenSize;
				f3TempNormal     =  texture(uBilateralNormalTex, f2TexCoord1).xyz;
				f3SampleNormal[1]  = transformNormalBack(f3TempNormal);
			}

			if ((y+1 < uScreenSize.y) && (iGridY+uSampleOffset < uScreenSize.y))
			{
				vec2 f2TexCoord2 = vec2(iGridX, uSampleOffset+iGridY) / uScreenSize;
				f3TempNormal     = texture(uBilateralNormalTex, f2TexCoord2).xyz;
				f3SampleNormal[2]  = transformNormalBack(f3TempNormal);
			}

			if ((x+1 < uScreenSize.x) && (y+1 < uScreenSize.y) && (iGridX+uSampleOffset < uScreenSize.x) && (iGridY+uSampleOffset < uScreenSize.y))
			{
				vec2 f2TexCoord3 = vec2(iGridX+uSampleOffset, iGridY+uSampleOffset) / uScreenSize;
				f3TempNormal     = texture(uBilateralNormalTex, f2TexCoord3).xyz;
				f3SampleNormal[3]  = transformNormalBack(f3TempNormal);
			}

			vec3 f3Normal01 = f3SampleNormal[0] * (1.0f - fDistPercentX) + f3SampleNormal[1] * fDistPercentX;
			vec3 f3Normal23 = f3SampleNormal[2] * (1.0f - fDistPercentX) + f3SampleNormal[3] * fDistPercentY;
			f3Normal = f3Normal01 * (1.0 - fDistPercentY) + f3Normal23 * fDistPercentY;
			f3Normal = normalize(f3Normal);
			f3Normal = f3Normal * 0.5 + vec3(0.5);
		}
	}

	_ShadedNormal = vec4(f3Normal, 1.0);

// calculate fluid color
	float Diffuse = max(0.0, dot(f3Normal, uLightDir));
	_FluidColor = vec4(uFluidColor*Diffuse, 1.0);
	//_FluidColor = vec4(1.0);
}