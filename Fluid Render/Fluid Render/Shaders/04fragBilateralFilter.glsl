#version 450

layout (location = 0) out vec4 _BilateralFilterNormal;
in vec2 g_TexCoord;

uniform sampler2D uFluidDepthTex;
uniform vec2 uScreenSize;
uniform float uFOV;

//--------------------------------------------------------------------------------------
// z值在x方向的偏导数
//--------------------------------------------------------------------------------------
float dz2x(in float x, in float y)
{
	if (x<=0 || y<=0 || x>=1 || y>=1) return 0;

	float fLeft		  = textureOffset(uFluidDepthTex, vec2(x, y), ivec2(-1, 0)).r;
	float fFluidDepth = texture(uFluidDepthTex, vec2(x, y)).r;
	float fRight      = textureOffset(uFluidDepthTex, vec2(x, y), ivec2(1, 0)).r;

	float Ret = 0.0;
	if (fLeft == 0.0 && fRight != 0.0)
		Ret = fRight - fFluidDepth;
	else if (fRight == 0.0 && fLeft != 0.0)
		Ret = fFluidDepth - fLeft;
	else
		Ret = (fRight - fLeft) / 2.0f;

	return Ret;
}

//--------------------------------------------------------------------------------------
// z值在y方向的偏导数
//--------------------------------------------------------------------------------------
float dz2y(in float x, in float y)
{
	if (x<=0 || y<=0 || x>=1 || y>=1) return 0;

	float fDescend	  = textureOffset(uFluidDepthTex, vec2(x, y), ivec2(0, -1)).r;
	float fFluidDepth = texture(uFluidDepthTex, vec2(x, y)).r;
	float fUpper      = textureOffset(uFluidDepthTex, vec2(x, y), ivec2(0, 1)).r;

	float Ret = 0.0;
	if (fDescend == 0.0 && fUpper != 0.0)
		Ret = fUpper - fFluidDepth;
	else if (fUpper == 0.0 && fDescend != 0.0)
		Ret = fFluidDepth - fDescend;
	else
		Ret = (fUpper - fDescend) / 2.0f;

	return Ret;
}

//--------------------------------------------------------------------------------------
// main function
//NOTE: dx表示z在x方向的偏导数; dx2表示前者的平方; dxx表示z在x方向的二次偏导, 
//--------------------------------------------------------------------------------------
void main()
{
	vec2 f2ViewSpaceCoord = gl_FragCoord.xy;
	vec2 f2TexCoord       = f2ViewSpaceCoord / uScreenSize;
	float fOffsetX        = 1.0 / uScreenSize.x;
	float fOffsetY        = 1.0 / uScreenSize.y;

// read eye-space depth from texture
	float fFluidDepth = texture(uFluidDepthTex, g_TexCoord).r;
	int SampleOffset = 6;
	int Counter = 0;
	vec3 f3Normal = vec3(0.0, 0.0, 0.0);

	if (fFluidDepth > 0.0)
	{
// bilateral filter
		for (int i=-SampleOffset; i<=SampleOffset; i++)
		{
			for (int k=-SampleOffset; k<=SampleOffset; k++)
			{
				float fCurrentDepth = textureOffset(uFluidDepthTex, f2TexCoord, ivec2(i, k)).r;
				float fx = gl_FragCoord.x + i;
				float fy = gl_FragCoord.y + k;

				if (fx>=0.0 && fx<=uScreenSize.x && fy>=0.0 && fy<=uScreenSize.y && fCurrentDepth>0.0)
				{
					float dx = dz2x(f2TexCoord.x + i*fOffsetX, f2TexCoord.y + k*fOffsetY);
					float dy = dz2y(f2TexCoord.x + i*fOffsetX, f2TexCoord.y + k*fOffsetY);

// constants
					const float Cx = fx == 0.0 ? 0.0 : 2.0 / (fx * tan(uFOV / 2.0));
					const float Cy = fy == 0.0 ? 0.0 : 2.0 / (fy * tan(uFOV / 2.0));
					//const float Cx = fx == 0.0 ? 0.0 : 2.0 / (fx * uScreenSize.x);
					//const float Cy = fy == 0.0 ? 0.0 : 2.0 / (fy * uScreenSize.y);

					const float dx2 = dx * dx;
					const float dy2 = dy * dy;
					const float Cx2 = Cx * Cx;
					const float Cy2 = Cy * Cy;

					float D = Cy2*dx2 + Cx2*dy2 + Cx2*Cy2*fFluidDepth*fFluidDepth;
					if (D == 0) continue;

					float inv_sqrtD = 1.0 / sqrt(D);
					f3Normal.x += Cy * dx * inv_sqrtD;
					f3Normal.y += Cx * dy * inv_sqrtD;
					f3Normal.z += Cx * Cy * fCurrentDepth * inv_sqrtD;
					Counter++;
				}

			}//end for1
		}//end for2

		f3Normal = f3Normal / float(Counter);
		f3Normal = normalize(f3Normal);
		f3Normal = f3Normal * 0.5 + vec3(0.5);
	}//end if

	_BilateralFilterNormal = vec4(f3Normal, 1.0);
}