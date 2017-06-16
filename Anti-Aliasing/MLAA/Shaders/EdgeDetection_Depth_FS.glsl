#version 450

in vec2 _TexCoord;

uniform sampler2D uDepthTex;
uniform	vec2 uTexelSize = vec2(1.0f/1280, 1.0f/720);

const float c_THRESHOLD = 0.1f;

layout(location = 0) out vec4 EdgeDetectionColor;

void main()
{
	vec2 TexCoord = vec2(_TexCoord.x, _TexCoord.y);

	float D =       texture2D(uDepthTex, TexCoord).r;
	float Dleft =   texture2D(uDepthTex, TexCoord - vec2(uTexelSize.x, 0.0f)).r;
	float Dtop =    texture2D(uDepthTex, TexCoord + vec2(0.0f, uTexelSize.y)).r;
	float Dright =  texture2D(uDepthTex, TexCoord + vec2(uTexelSize.x, 0.0f)).r;
	float Dbottom = texture2D(uDepthTex, TexCoord - vec2(0.0f, uTexelSize.y)).r;
	
	vec4 Delta = abs(vec4(D, D, D, D) - vec4(Dleft, Dtop, Dright, Dbottom));
	vec4 Edges = step(vec4(c_THRESHOLD)/10.0f, Delta);//小于阈值返回0.0.不然返回1.0
	
	if (dot(Edges, vec4(1.0)) == vec4(0.0))
	{
		discard;
	}

	EdgeDetectionColor = Edges;
	//EdgeDetectionColor  = vec4(1.0, 0.0, 0.0, 1.0);
}