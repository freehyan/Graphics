#version 450 core

layout (location = 0) out vec4 _FluidThickness;

uniform mat4 uProjectionMatrix;
uniform float uPointRadius;
uniform float uPointSize = 1;

//in vec3 g_EyeSpacePos;

void main()
{
// calculate eye-space sphere normal from texture coordinates
	vec3 f3FluidNormal;
	f3FluidNormal.xy = gl_PointCoord .xy * vec2(2.0f, -2.0f) + vec2(-1.0f, 1.0f);
	float fRadius = dot(f3FluidNormal.xy, f3FluidNormal.xy);

	if (fRadius > 1.0) discard;
	f3FluidNormal.z = sqrt(1.0 - fRadius);

//calculate fluid thickness with exponential falloff based on radius
	float fThickness = f3FluidNormal.z * 0.005; //???
//	float fThickness = f3FluidNormal.z * uPointSize * 2.0f * exp(-fRadius * 2.0f);
	_FluidThickness = vec4(fThickness, fThickness, fThickness, 1.0f);
}