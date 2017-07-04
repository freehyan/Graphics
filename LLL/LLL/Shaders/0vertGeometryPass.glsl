#version 450 core

layout (location = 0) in vec3 _Position;
layout (location = 1) in vec2 _TexCoord;                                             
layout (location = 2) in vec3 _Normal;   

uniform mat4 _uMVPMatrix;
uniform mat4 _uModelMatrix;
uniform mat4 _uNormalMatrix;

out	vec2 TexCoord_;
out vec3 Normal_;
out vec3 WorldPos_;

void main()
{
	gl_Position = _uMVPMatrix * vec4(_Position, 1.0);
	TexCoord_ = _TexCoord;
	WorldPos_ = vec3(_uModelMatrix * vec4(_Position, 1.0)).xyz;
	Normal_ = vec3(_uNormalMatrix * vec4(_Normal, 1.0)).xyz;
}