#version 450 core

//layout (early_fragment_tests) in;

layout (binding = 0, r32ui) uniform uimage2D uStartOffsetImage;
layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer uFragmentListBuffer;

layout (binding = 0, offset = 0) uniform atomic_uint uFragmentListCounter;

layout (location = 0) out vec4 Color;

//struct FragmentLinkedListBuffer
//{
//	uint uPixelColor;
//	float fDepth;
//	uint uNext;
//};

in vec2 TexCoords;
uniform sampler2D uColorTex;

uniform vec3 uLightDirectionE = vec3(1, 1, 1);
uniform vec3 uMaterialAmbient = vec3(0.3, 0.3, 0.3);
uniform vec3 uMaterialDiffuse = vec3(0.9, 0.9, 0.9);
uniform vec3 uLightAmbient = vec3(0.6, 0.6, 0.6);
uniform vec3 uLightDiffuse = vec3(0.9, 0.9, 0.9);

in vec3  _NormalE;

void main()
{
	uint Index;
	uint OldStartOffset;
	uvec4 Fragment;

	vec4 FragColor = vec4(0.0);
	//暂时只用物体材质颜色作测试
	//FragColor = texture2D(uColorTex, TexCoords);
	vec3 Ambient = uLightAmbient * uMaterialAmbient;
	FragColor = vec4(Ambient, 1);
	
	vec3 Normal = normalize(_NormalE);
	float t = dot(normalize(uLightDirectionE), Normal);
	if (t > 0.0)
	{
		vec3 DiffuseColor = uLightDiffuse * uMaterialDiffuse * t;
		FragColor.xyz += DiffuseColor;
	}
	FragColor = FragColor*0.5  + 0.5*texture2D(uColorTex, TexCoords);

	Index = atomicCounterIncrement(uFragmentListCounter);
	OldStartOffset = imageAtomicExchange(uStartOffsetImage, ivec2(gl_FragCoord.xy), uint(Index));

	Fragment.x = OldStartOffset;
	Fragment.y = packUnorm4x8(FragColor); //浮点打包，每个标准化的浮点元素转换成8位的整数值，然后结果打包为32位无符号整数。round(clamp(c, 0, _1) * 255.0)。clamp返回中间值，round返回一个最接近x的整数值
	Fragment.z = floatBitsToUint(gl_FragCoord.z); //返回浮点数编码的无符号的整数值
	Fragment.w = 0;

	imageStore(uFragmentListBuffer, int(Index), Fragment);
	//Color = FragColor;
	//Color = vec4(1, 0.0, 0.0, 1.0);
}