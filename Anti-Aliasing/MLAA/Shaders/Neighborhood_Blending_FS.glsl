#version 450

in vec2 _TexCoord;


uniform sampler2D uColorTex;
uniform sampler2D uBlendingWeightTex;
uniform	vec2 uTexelSize = vec2(1.0f/1280, 1.0f/720);

layout(location = 0) out vec4 FS_FinalColor;

vec4 mad(vec4 m, vec4 a, vec4 b)
{
	return m * a + b;
}

void main()
{
	vec2 TexCoord = vec2(_TexCoord.x, _TexCoord.y);

	vec4 TopLeft = texture2D(uBlendingWeightTex, _TexCoord );
	float Bottom = texture2D(uBlendingWeightTex, _TexCoord - vec2(0.0f, uTexelSize.y)).g;
	float Right  = texture2D(uBlendingWeightTex, _TexCoord + vec2(uTexelSize.x, 0.0f)).a;
	vec4 A = vec4(TopLeft.r, Bottom, TopLeft.b, Right);

	vec4 W = A * A * A;
	float Sum = dot(W, vec4(1.0));
	if (Sum > 0.0f)
	{
		vec4 Color = vec4(0.0);
		vec4 C = texture2D(uColorTex, TexCoord);
		vec4 Cleft =   texture2D(uColorTex, TexCoord - vec2(uTexelSize.x, 0.0f));
		vec4 Ctop =    texture2D(uColorTex, TexCoord +  vec2(0.0f, uTexelSize.y));
		vec4 Cright =  texture2D(uColorTex, TexCoord + vec2(uTexelSize.x, 0.0f));
		vec4 Cbottom = texture2D(uColorTex, TexCoord - vec2(0.0f, uTexelSize.y));
			
		Color = mix(C, Ctop, A.r) * W.r + Color;
		Color = mix(C, Cbottom, A.g) * W.g + Color;
		Color = mix(C, Cleft, A.b) * W.b + Color;
		Color = mix(C, Cright, A.a) * W.a + Color;	
			
		FS_FinalColor = Color / Sum;
	}
	else
		FS_FinalColor = texture2D(uColorTex, TexCoord);
}