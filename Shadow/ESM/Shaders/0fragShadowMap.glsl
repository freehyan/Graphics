#version 450 core

layout (location = 0) out vec4 FragColor_;

in float g_DepthZ;
//in float g_DepthW;
uniform int uAlpha; 

void main()
{
	float DepthZ = 0.5 * g_DepthZ  + 0.5;
	//float DepthZ = 0.5 *(g_DepthZ / g_DepthW) + 0.5;

	float ESMDpethZ = exp(uAlpha * DepthZ);
	FragColor_ = vec4(ESMDpethZ, ESMDpethZ, ESMDpethZ, 1.0);
}