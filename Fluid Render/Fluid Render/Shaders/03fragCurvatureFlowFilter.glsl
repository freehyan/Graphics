
#version 450

layout (location = 0) out vec4 _CurtureFlowFilterDepth;
in vec2 g_TexCoord;

uniform sampler2D uFluidDepthTex;
uniform vec2 uScreenSize;
uniform float uFOV;
uniform float fEpsilon = 0.01f;
uniform float fCFThreshold = 1.0f;

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
	if (fFluidDepth == 0.0) return Ret;

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
	if (fFluidDepth == 0.0) return Ret;

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

	if (fFluidDepth > 0.0)
	{
		float dx      = dz2x(f2TexCoord.x, f2TexCoord.y);
		float dxLeft  = dz2x(f2TexCoord.x - fOffsetX, f2TexCoord.y);
		float dxRight = dz2x(f2TexCoord.x + fOffsetX, f2TexCoord.y);
		float dxx     = (dxRight - dxLeft) / 2.0f;

		float dy         = dz2y(f2TexCoord.x, f2TexCoord.y);
		float dyDescend  = dz2y(f2TexCoord.x, f2TexCoord.y - fOffsetY);
		float dyUpper    = dz2y(f2TexCoord.x, f2TexCoord.y + fOffsetY);
		float dyy        = (dyUpper - dyDescend) / 2.0f;

// constants
		const float Cx = f2ViewSpaceCoord.x == 0.0 ? 0.0 : 2.0 / (f2ViewSpaceCoord.x * tan(uFOV / 2.0));
		const float Cy = f2ViewSpaceCoord.y == 0.0 ? 0.0 : 2.0 / (f2ViewSpaceCoord.y * tan(uFOV / 2.0));
		//const float Cx = f2ViewSpaceCoord.x == 0.0 ? 0.0 : 2.0 / (f2ViewSpaceCoord.x * uScreenSize.x);
		//const float Cy = f2ViewSpaceCoord.y == 0.0 ? 0.0 : 2.0 / (f2ViewSpaceCoord.y * uScreenSize.y);

		const float dx2 = dx * dx;
		const float dy2 = dy * dy;
		const float Cx2 = Cx * Cx;
		const float Cy2 = Cy * Cy;

// calculate curvature
		float D = Cy2*dx2 + Cx2*dy2 + Cx2*Cy2*fFluidDepth*fFluidDepth;
		float inv_D32 = 1.0f/ pow(D, 3.0f/2.0f);
	
		float kx = 4.0 / (uScreenSize.x * uScreenSize.x);
		float ky = 4.0 / (uScreenSize.y * uScreenSize.y);
		float dD_x = ky * pow(f2ViewSpaceCoord.y, -2.0f) * 2 * dx * dxx +
					 kx * dy2 * (-2) * pow(f2ViewSpaceCoord.x, -3.0f) +
					 2 * ky * pow(f2ViewSpaceCoord.y, -2.0f) * kx * (-1 * pow(f2ViewSpaceCoord.x, -3) * fFluidDepth * fFluidDepth + pow(f2ViewSpaceCoord.x, -2) * fFluidDepth * dx);

		float dD_y = ky * (-2) * pow(f2ViewSpaceCoord.y, -3) * dx * dx + 
					 kx * pow(f2ViewSpaceCoord.x, -2) * 2 * dy * dyy +
					 2 * kx * ky * pow(f2ViewSpaceCoord.x, -2) * (-1 * pow(f2ViewSpaceCoord.y, -3) * fFluidDepth * fFluidDepth + pow(f2ViewSpaceCoord.y, -2)  * fFluidDepth * dy);

		float Ex = 0.5 * dx * dD_x - dxx * D;
		float Ey = 0.5 * dy * dD_y - dyy * D;

		float H = (Cy * Ex + Cx * Ey) * inv_D32 / 2.0f;
		float fCF = H;
		fCF = fCF > fCFThreshold ? fCFThreshold : fCF;
		fCF = fCF < -fCFThreshold ? -fCFThreshold : fCF;
		//float H = Cy*dxx*D - Cy*dx*(Cy2*dx*dxx + Cx2*Cy2*fFluidDepth*dx) + Cx*dyy*D - Cx*dy*(Cx2*dy*dyy + Cx2*Cy2*fFluidDepth*dy);
		//H /= pow(D, 3.0f/2.0f);

// curvature dependent shift
		//const  float fEpsilon = 0.01;
		//float fCurtureFlowDepth = fFluidDepth + fEpsilon * H;
		float fCurtureFlowDepth = fFluidDepth - fEpsilon * fCF;
		if (fCurtureFlowDepth <= 0.0) fCurtureFlowDepth = 0.001;
	//	if (fCurtureFlowDepth >= 1.0) fCurtureFlowDepth = 0.999;

		_CurtureFlowFilterDepth = vec4(fCurtureFlowDepth, fCurtureFlowDepth, fCurtureFlowDepth, 1.0);
	}
}