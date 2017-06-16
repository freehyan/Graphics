
in vec4 vs_ProjPos;

layout(location = 0) out vec4 outColor;

uniform int uLightingModel;

uniform sampler2DMS uNormalDepthTex;
uniform sampler2DMS uDiffuseEdgeTex;
uniform usampler2DMS uCoverageTex;
uniform sampler2DRect uResolvedColorTex;

uniform mat4 uProjViewMatrix;
uniform mat4 uViewWorldMatrix;
const float cRoughness = 0.8;

void main(void)
{
    vec3 diffuse = vec3(0.0);
    vec3 dirToLight = normalize(vec3(-1, 1, -1));
    vec3 lightColor = vec3(1, 1, 1);

    // Read G-Buffer
    vec4 gBuf = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy), gl_SampleID);
    vec3 worldNormal = gBuf.xyz;
    float viewLinearDepth = gBuf.w;
    
    vec3 mtlColor = texelFetch(uDiffuseEdgeTex, ivec2(gl_FragCoord.xy), gl_SampleID).rgb;

    // Calculate position
    vec3 viewPos = DepthToPosition(viewLinearDepth, vs_ProjPos, uProjViewMatrix, 100.0f);
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
            break;
        }
    }
    else
    {
        diffuse += vec3(0.2, 0.5, 1.0);
    }

    outColor = vec4(diffuse, 1.0);
}
