#version 450

layout (location=0) out vec4 FS_Color;

uniform sampler2DMS uNormalDepthTex;
uniform sampler2DMS uDiffuseEdgeTex;
uniform sampler2DRect uResolvedColorTex;
uniform int uUseDiscontinuity;

const uint cMSAACount = 4;

// Coverage Mask version
void main(void)
{
    if (uUseDiscontinuity > 0.5) //通过比较模型的深度或者法线或者颜色来检查物体边缘
    {
        vec4 NormalDepth = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), 0);
        vec4 DiffuseEdge = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), 0);

        vec3 Normal = NormalDepth.xyz;
        float Depth = NormalDepth.w;
        vec3 Albedo = DiffuseEdge.rgb;

        for (int i = 1; i < cMSAACount; ++i)
        {
            vec4 NormalDepthNext = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), i);
            vec4 DiffuseEdgeNext = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), i);

            vec3 NextNormal = NormalDepthNext.xyz;
            float NextDepth = NormalDepthNext.w;
            vec3 NextAlbedo = DiffuseEdgeNext.rgb;

			//只要满足一个条件，即为Complex Pixel
            if (abs(Depth - NextDepth) > 0.1f ||
                abs(dot(abs(Normal - NextNormal), vec3(1, 1, 1))) > 0.1f ||
                abs(dot(Albedo - NextAlbedo, vec3(1, 1, 1))) > 0.1f)
            {
                discard;
            }
        }
    }
    else if (texture(uResolvedColorTex, gl_FragCoord.xy).a > 0.02) //a通道的值为0和1，1为Complex Pixel，0为Simple Pixel
    {
        discard;
    }

    FS_Color = vec4(0.0, 0.0, 0.0, 0.0); //Simple Pixel通过模板测试，模板值设置为1
}
