#version 450 core

layout (location = 0) out vec4 _FluidDepth;

uniform mat4 uProjectionMatrix;

uniform float uPointRadius;

in vec3 g_EyeSpacePos;

void main()
{
// calculate eye-space sphere normal from texture coordinates
	vec3 FluidNormal;
	FluidNormal.xy = gl_PointCoord .xy * vec2(2.0f, -2.0f) + vec2(-1.0f, 1.0f);
	float Radius = dot(FluidNormal.xy, FluidNormal.xy);
	if (Radius > 1.0) discard;

	FluidNormal.z = sqrt(1.0 - Radius);

// calculate fluid depth
	vec4 FragmentPos = vec4(g_EyeSpacePos + FluidNormal * uPointRadius * 0.8, 1.0);

//NDC	
	vec4 ClipSpacePos = uProjectionMatrix * FragmentPos;
	float Depth = ClipSpacePos.z / ClipSpacePos.w;
	float NormalDepth = Depth * 0.5 + 0.5;
	gl_FragDepth = NormalDepth;

	//_FluidDepth = vec4(Depth, Depth, Depth, 1.0);
	_FluidDepth = vec4(-FragmentPos.z, -FragmentPos.z, -FragmentPos.z, 1.0);
}