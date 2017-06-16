
in vec4 vs_ProjPos;

layout(location = 0) out vec4 outColor;

uniform bool uSeparateEdgePass;
uniform bool uSecondPass;
uniform bool uUseDiscontinuity;
uniform bool uAdaptiveShading;
uniform bool uShowComplexPixels;
uniform int uLightingModel;
uniform int uSampleMask;

uniform sampler2DMS uNormalDepthTex;
uniform sampler2DMS uDiffuseEdgeTex;
uniform usampler2DMS uCoverageTex;
uniform sampler2DRect uResolvedColorTex;

uniform mat4 uProjViewMatrix;
uniform mat4 uViewWorldMatrix;

const float cRoughness = 0.8;
const uint cMSAACount = 4;

void PerformFragmentAnalysis(in uvec4 inputCoverage, out uint sampleCount, out uint fragmentCount, out uvec4 sampleWeights)
{
    uvec4 coverage = inputCoverage;

    sampleWeights = uvec4(1);

    // Kill the same primitive IDs in the pixel
    if (coverage.x == coverage.y) { coverage.y = 0; sampleWeights.x += 1; sampleWeights.y = 0; }
    if (coverage.x == coverage.z) { coverage.z = 0; sampleWeights.x += 1; sampleWeights.z = 0; }
    if (coverage.x == coverage.w) { coverage.w = 0; sampleWeights.x += 1; sampleWeights.w = 0; }
    if (coverage.y == coverage.z) { coverage.z = 0; sampleWeights.y += 1; sampleWeights.z = 0; }
    if (coverage.y == coverage.w) { coverage.w = 0; sampleWeights.y += 1; sampleWeights.w = 0; }
    if (coverage.z == coverage.w) { coverage.w = 0; sampleWeights.z += 1; sampleWeights.w = 0; }

    sampleWeights.x = coverage.x > 0 ? sampleWeights.x : 0;
    sampleWeights.y = coverage.y > 0 ? sampleWeights.y : 0;
    sampleWeights.z = coverage.z > 0 ? sampleWeights.z : 0;
    sampleWeights.w = coverage.w > 0 ? sampleWeights.w : 0;

    sampleCount = (sampleWeights.x > 0 ? 1 : 0) + (sampleWeights.y > 0 ? 1 : 0) + (sampleWeights.z > 0 ? 1 : 0) + (sampleWeights.w > 0 ? 1 : 0);
    fragmentCount = sampleWeights.x + sampleWeights.y + sampleWeights.z + sampleWeights.w;
}

// Coverage Mask, no separate pass version
void main(void)
{
    bool bComplexPixel = false;

    if (uSeparateEdgePass)
    {
        if (uSecondPass)//Simple pixel 和complex pixel分两个pass渲染
        {
            bComplexPixel = true;
        }
    }
    else 
    {
        if (uUseDiscontinuity) //不分两个pass渲染，但是使用检测不连续边缘方法
        {
            vec4 gBuf1 = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), 0);
            vec4 gBuf2 = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), 0);

            vec3 normal = gBuf1.xyz;
            float depth = gBuf1.w;
            vec3 albedo = gBuf2.rgb;

            for (int i = 1; i < cMSAACount; ++i)
            {
                vec4 gBuf1next = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), i);
                vec4 gBuf2next = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), i);

                vec3 nextNormal = gBuf1next.xyz;
                float nextDepth = gBuf1next.w;
                vec3 nextAlbedo = gBuf2next.rgb;

                if (abs(depth - nextDepth) > 0.1f ||
                    abs(dot(abs(normal - nextNormal), vec3(1, 1, 1))) > 0.1 ||
                    abs(dot(albedo - nextAlbedo, vec3(1, 1, 1))) > 0.1)
                {
                    bComplexPixel = true;
                    break;
                }
            }
        }
        else
        {
            bComplexPixel = texture(uResolvedColorTex, gl_FragCoord.xy).a > 0.02;
        }
    }

    uint sampleCount;
    uint fragmentCount;
    vec3 diffuse = vec3(0.0);
    vec3 dirToLight = normalize(vec3(-1, 1, -1));
    vec3 lightColor = vec3(1, 1, 1);

    if (bComplexPixel) 
    {
        sampleCount = cMSAACount;
        fragmentCount = cMSAACount;
        uvec4 sampleWeights = uvec4(1);

        if (uAdaptiveShading)
        {
            uvec4 coverage;
            for (int i = 0; i < cMSAACount; ++i)
            { 
                coverage[i] = texelFetch(uCoverageTex, ivec2(gl_FragCoord.xy), i).r;
            }
            PerformFragmentAnalysis(coverage, sampleCount, fragmentCount, sampleWeights);
        }

        int samplesProcessed = 0;
        for (int sampleIndex = 0;  sampleIndex < cMSAACount; ++sampleIndex)
        {
            if (uAdaptiveShading)
            {
                if (samplesProcessed == sampleCount)
                {
                    break;
                }

                if (sampleWeights[sampleIndex] == 0)
                {
                    continue;
                }
            }

            // Read G-Buffer
            vec4 gBuf = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), sampleIndex);
            vec3 mtlColor = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), sampleIndex).rgb;

            // Get the normal
            vec3 worldNormal = gBuf.xyz;

            // Calculate position
            float viewLinearDepth = gBuf.w;
            vec3 viewPos = DepthToPosition(viewLinearDepth, vs_ProjPos, uProjViewMatrix, 1000.0f);
            vec3 worldPos = (uViewWorldMatrix * vec4(viewPos, 1.0f)).xyz;

            // Calculate to eye vector
            vec3 eyeWorldPos = (uViewWorldMatrix * vec4(0, 0, 0, 1)).xyz;
            vec3 toEyeVector = normalize(eyeWorldPos - worldPos);
            float normLength = length(worldNormal);
            vec3 sampleDiffuse;
            if (normLength > 0.0)
            {
                vec3 normal = worldNormal / normLength;
                switch (uLightingModel)
                {
                case 1:
                    sampleDiffuse = OrenNayar(dirToLight, toEyeVector, normal, lightColor, mtlColor, cRoughness);
                    break;
                case 0:
                default:
                    sampleDiffuse = LambertDiffuse(dirToLight, normal, lightColor, mtlColor);
                    break;
                }
            }
            else
            {
                // No geometry rendered, so use sky color
                sampleDiffuse = vec3(0.2, 0.5, 1.0);
            }

            if (uAdaptiveShading)
            {
                sampleDiffuse *= sampleWeights[sampleIndex];
            }
            diffuse += sampleDiffuse;
            ++samplesProcessed;
        }
        
        outColor = vec4(diffuse / fragmentCount, 1.0f);
    }
    else
    {
        sampleCount = 1;
        fragmentCount = 1;

        // Read G-Buffer
        vec4 gBuf = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), 0);
        vec3 mtlColor = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), 0).rgb;

        // Get the normal
        vec3 worldNormal = gBuf.xyz;

        // Calculate position
        float viewLinearDepth = gBuf.w;
        vec3 viewPos = DepthToPosition(viewLinearDepth, vs_ProjPos, uProjViewMatrix, 1000.0f);
        vec3 worldPos = (uViewWorldMatrix * vec4(viewPos, 1.0f)).xyz;

        // Calculate to eye vector
        vec3 eyeWorldPos = (uViewWorldMatrix * vec4(0, 0, 0, 1)).xyz;
        vec3 toEyeVector = normalize(eyeWorldPos - worldPos);
        float normLength = length(worldNormal);
        if (normLength > 0.0)
        {
            vec3 normal = worldNormal / normLength;
            switch (uLightingModel)
            {
            case 1:
                diffuse += OrenNayar(dirToLight, toEyeVector, normal, lightColor, mtlColor, cRoughness);
                break;
            case 0:
            default:
                diffuse += LambertDiffuse(dirToLight, normal, lightColor, mtlColor);
			//	outColor = vec4(worldNormal, 1.0);
                break;
            }
        }
        else
        {
            // No geometry rendered, so use sky color
            diffuse += vec3(0.2, 0.5, 1.0);
        }
        outColor = vec4(diffuse, 1.0);
    }

    // If we're marking complex pixels, then we can just emit the appropriate color and early out
    if (uShowComplexPixels)
    {
        if (sampleCount == 2)
        {
            outColor = vec4(0.0, 0.5, 0.0, 1.0);
            return;
        }
        if (sampleCount == 3)
        {
            //outColor = vec4(0.5, 0.5, 0.0, 1.0);
			outColor = vec4(0.0, 0.0, 0.5, 1.0);
            return;
        }
        if (sampleCount >= 4)
        {
            outColor = vec4(0.5, 0.0, 0.0, 1.0);
            return;
        }
    }
}
