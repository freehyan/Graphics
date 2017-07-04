#version 450 core

layout (binding = 0, r32ui) uniform uimage2D uLightStartOffsetImage;
layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer uLightFragmentLinkedLBuffer;
layout (binding = 0, offset = 0) uniform atomic_uint uLLLCounter;

uniform uint uLightIndex;
uniform sampler2D uDepthTex;
uniform vec2 uScreenSize;

//暂时只分配光源前面深度值
void allocateLightFragmentLink(in float vLightMin, in uint vLightIndex)
{
	uint LightDepthMin = floatBitsToUint(vLightMin);
	uvec4 LightFragment;
	LightFragment.x = vLightIndex;
	LightFragment.y = LightDepthMin;
	LightFragment.z = 0;

	uint NewLLLIndex;
	uint PrevLLLIndex;

	NewLLLIndex = atomicCounterIncrement(uLLLCounter);
	//if (NewLLLIndex >= g_LLLMaxAlloc)
	//	return;
	
	PrevLLLIndex = imageAtomicExchange(uLightStartOffsetImage, ivec2(gl_FragCoord.xy), uint(NewLLLIndex));
	LightFragment.w = PrevLLLIndex;

	imageStore(uLightFragmentLinkedLBuffer, int(NewLLLIndex), LightFragment);
}

void insertLightCullingBackface(in vec2 vScreenTexPos, in uint vLightIndex, in bool vIsFrontFace)
{
	float fSceneDepth = texture2D(uDepthTex, vScreenTexPos).x;
	float fLightDepth = gl_FragCoord.z;

	if (vIsFrontFace == true)
	{
		if (fSceneDepth < fLightDepth) //光源球被场景遮挡
			return;
	}

	allocateLightFragmentLink(fLightDepth, vLightIndex);
}

vec2 calcTexCoords()
{
	return gl_FragCoord.xy / uScreenSize;
}

void main()
{	
	bool bFrontFace = gl_FrontFacing;
	vec2 ScreenTexPos = calcTexCoords(); //0 -1

	insertLightCullingBackface(ScreenTexPos, uLightIndex, bFrontFace);
}