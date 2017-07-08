#version 450 core
                                                  
const int MAX_SPOT_LIGHTS = 1;     

in VS_OUT{
	vec4 LightSpacePos;
	vec2 TexCoord;
	vec3 WorldPos;
	vec3 Normal;
}fs_in;

struct SBaseLight 
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct SAttenuation
{
	float Constant;
	float Linear;
	float Quadratic;
};

struct SPointLight
{
	SBaseLight BaseLight;
	vec3 LightPosition;
	SAttenuation Attenuation;
};

struct SSpotLight                                                                            
{                                                                                           
    SPointLight PointLight;                                                                 
    vec3 Direction;                                                                         
    float Cutoff;                                                                           
};  
                                                            
uniform uint uSpotLightNum;                                                                                                    
uniform SSpotLight uSpotLights[MAX_SPOT_LIGHTS];     

uniform sampler2D uPrefilterTextureZ;
uniform sampler2D uColorTexture;

uniform vec3 uEyeWorldPos;                                                                  
uniform float uMatSpecularIntensity = 1.0;                                                        
uniform float uSpecularPower = 128;   
uniform int uAlpha; 

layout (location = 0) out vec4 FragColor_;

//***********************************************************
//FUNCTION:
float calcShadowFactor(in vec4 vLightSpacePos)                                                  
{                                                                                           
    vec3 ProjCoords = vLightSpacePos.xyz / vLightSpacePos.w;  
	ProjCoords = ProjCoords*0.5 + 0.5;
		                                                                      
    float ClosestExponentialDepth = texture(uPrefilterTextureZ, ProjCoords.xy).r;                                     
	float CurrentDepth = ProjCoords.z;
	float CurrentExponentialDepth = exp(-uAlpha * CurrentDepth);

	float Shadow = CurrentExponentialDepth * ClosestExponentialDepth;
	return Shadow;                                                                      
}   

//***********************************************************
//FUNCTION:
vec4 calcLightInternal(in SBaseLight vBaseLight, in vec3 vLightDirection, in vec3 vNormal, in float vShadowFactor)                                                  
{                                                                                           
    vec4 AmbientColor = vec4(vBaseLight.Color * vBaseLight.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(vNormal, -vLightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(vBaseLight.Color * vBaseLight.DiffuseIntensity * DiffuseFactor, 1.0f);    
                                                                                            
        vec3 VertexToEye = normalize(uEyeWorldPos - fs_in.WorldPos);                             
        vec3 LightReflect = normalize(reflect(vLightDirection, vNormal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                                      
        if (SpecularFactor > 0) {                                                           
            SpecularFactor = pow(SpecularFactor, uSpecularPower);                               
            SpecularColor = vec4(vBaseLight.Color, 1.0f) * uMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor +  vShadowFactor * (DiffuseColor + SpecularColor));                  
}                                                                                           

//***********************************************************
//FUNCTION:
vec4 calcPointLight(in SPointLight vPointLight, in vec3 vNormal, in vec4 vLightSpacePos)                   
{                                                                                           
    vec3 LightDirection = fs_in.WorldPos - vPointLight.LightPosition;                                           
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
    float ShadowFactor = calcShadowFactor(vLightSpacePos);                                   
                                                                                            
    vec4 Color = calcLightInternal(vPointLight.BaseLight, LightDirection, vNormal, ShadowFactor);           
    float Attenuation =  vPointLight.Attenuation.Constant +                                                 
                         vPointLight.Attenuation.Linear * Distance +                                        
                         vPointLight.Attenuation.Quadratic * Distance * Distance;                                 
                                                                                            
    return Color / Attenuation;                                                             
}                                                                                           

//***********************************************************
//FUNCTION:                                                                                            
vec4 calcSpotLight(in SSpotLight vSpotLight, in vec3 vNormal, in vec4 vLightSpacePos)                     
{                                                                                           
    vec3 LightToPixel = normalize(fs_in.WorldPos - vSpotLight.PointLight.LightPosition);                             
    float SpotFactor = dot(LightToPixel, normalize(vSpotLight.Direction));                                      
                                                                                            
    if (SpotFactor > vSpotLight.Cutoff) {                                                            
        vec4 Color = calcPointLight(vSpotLight.PointLight, vNormal, vLightSpacePos);                         
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - vSpotLight.Cutoff));                   
    }                                                                                       
    else {                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}   

void main()
{
	vec3 Normal = normalize(fs_in.Normal);
	vec4 TotalLight = vec4(0.0);

	for (int i=0; i<uSpotLightNum; i++)
	{
		vec4 SpotLight = calcSpotLight(uSpotLights[i], Normal, fs_in.LightSpacePos);
		TotalLight += SpotLight;
	}

	vec4 SampledColor = texture(uColorTexture, fs_in.TexCoord);
	FragColor_ = TotalLight * SampledColor ;
}