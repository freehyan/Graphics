#version 450

in vec2 _TexCoord;

uniform sampler2D uPreComputedAreaTex;
uniform sampler2D uEdgeDetectionTex;
uniform	vec2 uTexelSize = vec2(1.0f/1280, 1.0f/720);

const int c_MAX_SEARCH_STEPS = 8;

#define NUM_DISTANCES 32
#define AREA_SIZE (NUM_DISTANCES * 5)

layout(location = 0) out vec4 FS_BlendingWeightColor;

vec4 mad(vec4 m, vec4 a, vec4 b)
{
	return m * a + b;
}

vec2 Area(vec2 vDistance, float e1, float e2)
{
	vec2 PixelCoord = NUM_DISTANCES * round(4.0 * vec2(e1, e2)) + vDistance;
	vec2 TexCoord   = PixelCoord / (AREA_SIZE - 1.0);
	
	return textureLod(uPreComputedAreaTex, TexCoord, 0.0).rg;
}

//向左迭代搜索边缘线的距离，采用双线性滤波方式
float searchXLeft(vec2 vTexCoord)
{
	vTexCoord -= vec2(1.5, 0.0) * uTexelSize;
	float e = 0.0f;
	
	int i = 0;
	// We offset by 0.5 to sample between edgels, thus fetching two in a row
	for (; i<c_MAX_SEARCH_STEPS; i++)
	{
		e = texture2D(uEdgeDetectionTex, vTexCoord).g;

		 //compare with 0.9 to prevent bilinear access precision problems
		if (e < 0.9) break;

		vTexCoord -= vec2(2.0, 0.0) * uTexelSize;
	}

	// When we exit the loop without founding the end, we want to return -2 * maxSearchSteps
	return max(-2.0 * i - 2.0 * e, -2.0 * c_MAX_SEARCH_STEPS);
}

float searchXRight(vec2 vTexCoord)
{
	vTexCoord += vec2(1.5, 0.0) * uTexelSize;
	float e = 0.0f;
	
	int i = 0;
	for (; i<c_MAX_SEARCH_STEPS; i++)
	{
		e = texture2D(uEdgeDetectionTex, vTexCoord).g;

		if (e < 0.9) break;

		vTexCoord += vec2(2.0, 0.0) * uTexelSize;
	}

	return min(2.0 * i + 2.0 * e, 2.0 * c_MAX_SEARCH_STEPS);
}

float searchYUp(vec2 vTexCoord)
{
	vTexCoord += vec2(0.0, 1.5) * uTexelSize;
	float e = 0.0f;

	int i = 0;
	for (; i<c_MAX_SEARCH_STEPS; i++)
	{
		e = texture2D(uEdgeDetectionTex, vTexCoord).r;

		if (e < 0.9) break;

		vTexCoord += vec2(0.0, 2.0) * uTexelSize;
	}
	return min(2.0 * i + 2.0 * e, 2.0 * c_MAX_SEARCH_STEPS);
}

float searchYDown(vec2 vTexCoord)
{
	vTexCoord -= vec2(0.0, 1.5) * uTexelSize;
	float e = 0.0f;

	int i = 0;
	for (; i<c_MAX_SEARCH_STEPS; i++)
	{
		e = texture2D(uEdgeDetectionTex, vTexCoord).r;

		if (e < 0.9) break;

		vTexCoord -= vec2(0.0, 2.0) * uTexelSize;
	}
	return max(-2.0 * i - 2.0 * e, -2.0 * c_MAX_SEARCH_STEPS);
}

void main()
{
	vec4 Weights = vec4(0.0);

	vec2 Edge = textureLod(uEdgeDetectionTex, _TexCoord, 0.0).rg;

	// Edge at north
	if (Edge.g > 0.0)
	{
		// Search distances to the left and to the right:
		vec2 DistanceLeftRight = vec2(searchXLeft(_TexCoord), searchXRight(_TexCoord));

		// Now fetch the crossing edges. Instead of sampling between edgels, we
		// sample at 0.25, to be able to discern what value has each edgel:
		vec4 RhombusesPos = vec4(DistanceLeftRight.x, 0.25, DistanceLeftRight.y + 1.0, 0.25);
		vec4 Coords = mad(RhombusesPos, vec4(uTexelSize.xy, uTexelSize.xy), vec4(_TexCoord.xy, _TexCoord.xy));

		float e1 = texture2D(uEdgeDetectionTex, Coords.xy).r;
		float e2 = texture2D(uEdgeDetectionTex, Coords.zw).r;

		Weights.rg = Area(abs(DistanceLeftRight), e1, e2);
	}

	if (Edge.r > 0.0) // Edge at west
	{
		// Search distances to the top and to the bottom:
		vec2 DistanceTopBottom = vec2(searchYUp(_TexCoord), searchYDown(_TexCoord));

		// Now fetch the crossing edges (yet again):
		vec4 RhombusesPos = vec4(-0.25, DistanceTopBottom.x, -0.25, DistanceTopBottom.y - 1.0);

		vec4 Coords = mad(RhombusesPos, vec4(uTexelSize.xy, uTexelSize.xy), vec4(_TexCoord.xy, _TexCoord.xy));

		float e1 = texture2D(uEdgeDetectionTex, Coords.xy).g;
		float e2 = texture2D(uEdgeDetectionTex, Coords.zw).g;

		Weights.ba = Area(abs(DistanceTopBottom), e1, e2);
	}

	FS_BlendingWeightColor = Weights;
}