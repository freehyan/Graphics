#version 450 core

layout (location = 0) in vec3 _Position;

uniform mat4 _uMVPMatrix;

void main()
{
	gl_Position = _uMVPMatrix * vec4(_Position, 1.0);
}