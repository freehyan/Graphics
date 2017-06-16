#version 450

in vec4 vs_ViewPos;
in vec3 vs_WorldNormal;
in vec2 vs_TexCoord;

layout(location = 0) out vec4 FS_NormalDepth;
layout(location = 1) out vec4 FS_Diffuse;

uniform vec3 uDiffuseColor = vec3(1.0f, 1.0f, 1.0f);

void main(void)
{
    FS_NormalDepth.xyz = normalize(vs_WorldNormal);
    FS_NormalDepth.w = vs_ViewPos.z / 1000.0; //视图空间的Z值除以远裁剪平面的距离 

    FS_Diffuse = vec4(uDiffuseColor, 1.0);
}