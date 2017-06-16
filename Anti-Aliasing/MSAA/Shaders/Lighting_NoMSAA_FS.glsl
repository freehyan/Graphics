
in vec4 vs_ProjPos;

layout(location = 0) out vec4 FS_Color;

uniform int uLightingModel;

uniform sampler2DRect uNormalDepthTex;
uniform sampler2DRect uDiffuseColorTex;

uniform mat4 uProjViewMatrix;
uniform mat4 uViewWorldMatrix;
const float cRoughness = 0.8;

const float cFarClip = 1000.0f;

void main(void)
{
    vec3 FragmentColor = vec3(0.0);

    vec4 NormalDepth = texelFetch(uNormalDepthTex, ivec2(gl_FragCoord.xy));
    vec3 DiffuseColor = texelFetch(uDiffuseColorTex, ivec2(gl_FragCoord.xy)).rgb;
  
	vec3 WorldNormal = NormalDepth.xyz;

    // Calculate position
    float ViewLinearDepth = NormalDepth.w;
    vec3 ViewPos = DepthToPosition(ViewLinearDepth, vs_ProjPos, uProjViewMatrix, cFarClip);
    vec3 WorldPos = (uViewWorldMatrix * vec4(ViewPos, 1.0f)).xyz;

    // Calculate to eye vector
    vec3 EyeWorldPos = (uViewWorldMatrix * vec4(0, 0, 0, 1)).xyz; //View空间下眼睛坐标为(0, 0, 0)
    vec3 toEyeVector = normalize(EyeWorldPos - WorldPos);
    float NormalLength = length(WorldNormal);
    if (NormalLength > 0.0)
    {
        vec3 Normal = WorldNormal / NormalLength;
        vec3 DirToLight = normalize(vec3(-1, 1, -1));
        vec3 LightColor = vec3(1, 1, 1);
        switch (uLightingModel)
        {
        case 1:
            FragmentColor += OrenNayar(DirToLight, toEyeVector, Normal, LightColor, DiffuseColor, cRoughness);
            break;
        case 0:
        default:
            FragmentColor += LambertDiffuse(DirToLight, Normal, LightColor, DiffuseColor);
            break;
        }
    }
    else
    {
        // No geometry rendered, so use sky color
        FragmentColor += vec3(0.2, 0.5, 1.0);
    }

    FS_Color = vec4(FragmentColor, 1.0);
}