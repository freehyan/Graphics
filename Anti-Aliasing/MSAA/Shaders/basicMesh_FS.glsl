#version 450

in vec4 vs_ViewPos;
in vec3 vs_WorldNormal;
in vec2 vs_TexCoord;

layout(location = 0) out vec4 outNormalDepth;
layout(location = 1) out vec4 outDiffuseEdge;
layout(location = 2) out uint outCoverage;

const uint cMSAACount = 4;
uniform int uSampleMask;
uniform vec3 uDiffuseColor = vec3(1.0f, 1.0f, 1.0f);

void main(void)
{
    outNormalDepth.xyz = normalize(vs_WorldNormal);
    outNormalDepth.w = vs_ViewPos.z / 1000.0;

    outDiffuseEdge.rgb = uDiffuseColor;
    outDiffuseEdge.a = (gl_SampleMaskIn[0] != uSampleMask) ? 1.0 : 0.0;

    // Max 32-bit MSAA
    outCoverage = (uSampleMask & gl_SampleMaskIn[0]); //eg: 1111 & 0011 = 0011 = 3
}