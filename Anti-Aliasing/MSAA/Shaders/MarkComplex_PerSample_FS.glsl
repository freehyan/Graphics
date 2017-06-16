#version 450

layout(location = 0) in vec4 inProjPos;

layout(location = 0) out vec4 outColor;

// Separate pass, marking only complex pixels
void main(void)
{
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
