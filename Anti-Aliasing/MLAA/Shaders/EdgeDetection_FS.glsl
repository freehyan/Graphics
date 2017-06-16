#version 450

in vec2 _TexCoord;

uniform sampler2D uColorTex;
uniform	vec2 uTexelSize = vec2(1.0f/1280, 1.0f/720);

const float c_THRESHOLD = 0.1f;

layout(location = 0) out vec4 EdgeDetectionColor;

void main()
{
	vec3 Weights = vec3(0.2126, 0.7152, 0.0722);

	vec2 TexCoord = vec2(_TexCoord.x, _TexCoord.y);

	float L =       dot( texture2D(uColorTex, TexCoord).xyz, Weights); //Luminance values 
	float Lleft =   dot( texture2D(uColorTex, TexCoord - vec2(uTexelSize.x, 0.0f) ).xyz, Weights);
	float Ltop =    dot( texture2D(uColorTex, TexCoord + vec2(0.0f, uTexelSize.y) ).xyz, Weights);
	float Lright =  dot( texture2D(uColorTex, TexCoord + vec2(uTexelSize.x, 0.0f) ).xyz, Weights);
	float Lbottom = dot( texture2D(uColorTex, TexCoord - vec2(0.0f, uTexelSize.y) ).xyz, Weights);
	
	vec4 Delta = abs(vec4(L, L, L, L) - vec4(Lleft, Ltop, Lright, Lbottom));
	vec4 Edges = step(vec4(c_THRESHOLD, c_THRESHOLD, c_THRESHOLD, c_THRESHOLD), Delta);//小于阈值返回0.0.不然返回1.0
	
	if (dot(Edges, vec4(1.0)) == vec4(0.0))
	{
		discard;
	}
	EdgeDetectionColor = Edges;
	//EdgeDetectionColor = vec4(Edges.xy, 0.0, 1.0);
}