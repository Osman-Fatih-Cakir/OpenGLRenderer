#version 450 core

in vec2 fTexCoord;

out vec4 outColor;

uniform vec2 resolution;
uniform sampler2D cur_depth_map;
uniform sampler2D prev_depth_map;
uniform sampler2D cur_color_map;
uniform sampler2D prev_color_map;
uniform sampler2D velocity_map;

const vec2 kOffsets3x3[9] =
{
	vec2(-1, -1),	// upper left
	vec2(0, -1),	// up
	vec2(1, -1),	// upper right
	vec2(-1,  0),	// left
	vec2(0,  0),	// K
	vec2(1,  0),	// right
	vec2(-1,  1),	// lower left
	vec2(0,  1),	// down
	vec2(1,  1),	// lower right
}; // k is index 4

// Number of neighbors.
const uint kNeighborsCount = 9;

const vec2 kOffsets2x2[5] =
{
	vec2(-1, 0),	// left
	vec2(0, -1),	// up
	vec2(0,  0),	// K
	vec2(1, 0),		// right
	vec2(0, 1)		// down
}; //k is index 2

const uint neighborCount = 5;

vec2 GetClosestUV(sampler2D depths)
{
	vec2 deltaRes = vec2(1.0 / resolution.x, 1.0 / resolution.y);
	vec2 closestUV = fTexCoord;
	float closestDepth = 1.0f;

	for (uint iter = 0; iter < kNeighborsCount; iter++)
	{
		vec2 newUV = fTexCoord + (kOffsets3x3[iter] * deltaRes);

		float depth = texture2D(depths, newUV).x;

		if (depth < closestDepth)
		{
			closestDepth = depth;
			closestUV = newUV;
		}
	}

	return closestUV;
}

vec2 MinMaxDepths(float neighborDepths[kNeighborsCount])
{
	float minDepth = neighborDepths[0];
	float maxDepth = neighborDepths[0];

	for (int iter = 1; iter < kNeighborsCount; iter++)
	{
		minDepth = min(minDepth, neighborDepths[iter]);
		minDepth = max(maxDepth, neighborDepths[iter]);
	}

	return vec2(minDepth, maxDepth);
}

vec4 MinColors(in vec4 neighborColors[neighborCount])
{
	vec4 minColor = neighborColors[0];

	for (int iter = 1; iter < neighborCount; iter++)
	{
		minColor = min(minColor, neighborColors[iter]);
	}

	return minColor;
}

vec4 MaxColors(in vec4 neighborColors[neighborCount])
{
	vec4 maxColor = neighborColors[0];

	for (int iter = 1; iter < neighborCount; iter++)
	{
		maxColor = max(maxColor, neighborColors[iter]);
	}

	return maxColor;
}

vec4 MinColors2(in vec4 neighborColors[kNeighborsCount])
{
	vec4 minColor = neighborColors[0];

	for (int iter = 1; iter < neighborCount; iter++)
	{
		minColor = min(minColor, neighborColors[iter]);
	}

	return minColor;
}

vec4 MaxColors2(in vec4 neighborColors[kNeighborsCount])
{
	vec4 maxColor = neighborColors[0];

	for (int iter = 1; iter < neighborCount; iter++)
	{
		maxColor = max(maxColor, neighborColors[iter]);
	}

	return maxColor;
}

// note: clips towards aabb center + p.w
vec4 clip_aabb(vec3 colorMin, vec3 colorMax, vec4 currentColor, vec4 previousColor)
{
	vec3 p_clip = 0.5 * (colorMax + colorMin);
	vec3 e_clip = 0.5 * (colorMax - colorMin);
	vec4 v_clip = previousColor - vec4(p_clip, currentColor.a);
	vec3 v_unit = v_clip.rgb / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
	if (ma_unit > 1.0)
	{
		return vec4(p_clip, currentColor.a) + v_clip / ma_unit;
	}
	else
	{
		return previousColor; // point inside aabb
	}
}

vec4 ConstrainHistory(vec4 neighborColors[neighborCount])
{
	vec4 previousColorMin = MinColors(neighborColors);
	vec4 previousColorMax = MaxColors(neighborColors);

	return clamp(neighborColors[2], previousColorMin, previousColorMax);
}

vec4 InsideResolve(vec2 velocity)
{
	vec2 deltaRes = vec2(1.0 / resolution.x, 1.0 / resolution.y);

	vec4 current3x3Colors[kNeighborsCount];
	vec4 previous3x3Colors[kNeighborsCount];

	for (uint iter = 0; iter < kNeighborsCount; iter++)
	{
		vec2 newUV = fTexCoord + (kOffsets3x3[iter] * deltaRes);

		current3x3Colors[iter] = texture2D(cur_color_map, newUV);

		previous3x3Colors[iter] = texture2D(prev_color_map, newUV + velocity);
	}

	vec4 rounded3x3Min = MinColors2(current3x3Colors);
	vec4 rounded3x3Max = MaxColors2(previous3x3Colors);

	vec4 current2x2Colors[neighborCount];
	vec4 previous2x2Colors[neighborCount];

	for (uint iter = 0; iter < neighborCount; iter++)
	{
		vec2 newUV = fTexCoord + (kOffsets2x2[iter] * deltaRes);

		current2x2Colors[iter] = texture2D(cur_color_map, newUV);

		previous2x2Colors[iter] = texture2D(prev_color_map, newUV + velocity);
	}

	vec4 min2 = MinColors(current2x2Colors);
	vec4 max2 = MaxColors(previous2x2Colors);

	// Mix the 3x3 and 2x2 min maxes together
	vec4 mixedMin = mix(rounded3x3Min, min2, 0.5);
	vec4 mixedMax = mix(rounded3x3Max, max2, 0.5);

	float testVel = 0.9 - (length(velocity) * 10.0);
	return mix(current2x2Colors[2], 
		clip_aabb(mixedMin.rgb, mixedMax.rgb, current2x2Colors[2], ConstrainHistory(previous2x2Colors)), testVel);
}

vec4 CustomResolve(float preNeighborDepths[kNeighborsCount], float curNeighborDepths[kNeighborsCount], vec2 velocity)
{
	// Use the closest depth instead?
	vec2 preMinMaxDepths = MinMaxDepths(preNeighborDepths);
	vec2 curMinMaxDepths = MinMaxDepths(curNeighborDepths);

	float highestDepth = min(preMinMaxDepths.x, curMinMaxDepths.x); //get the closest
	float lowestDepth = max(preMinMaxDepths.x, curMinMaxDepths.x); //get the furthest

	float depthFalloff = abs(highestDepth - lowestDepth);

	vec4 res = vec4(0);

	vec4 taa = InsideResolve(velocity);

	float averageDepth = 0;
	for (uint iter = 0; iter < kNeighborsCount; iter++)
	{
		averageDepth += curNeighborDepths[iter];
	}
	averageDepth /= kNeighborsCount;

	// For dithered edges, detect if the adge has been dithered? 
	// Use a 3x3 grid to see if anything around it has high enough depth?
	if (averageDepth < 1.0)
	{
		res = taa;
	}
	else
	{
		res = texture2D(cur_color_map, fTexCoord);
	}

	return res;
}

void main()
{
	float currentDepths[kNeighborsCount];
	float previousDepths[kNeighborsCount];

	vec2 deltaRes = vec2(1.0 / resolution.x, 1.0 / resolution.y);

	vec2 closestVec = -(texture2D(velocity_map, GetClosestUV(cur_depth_map)).rg);

	for (uint iter = 0; iter < kNeighborsCount; iter++)
	{
		vec2 newUV = fTexCoord + (kOffsets3x3[iter] * deltaRes);

		currentDepths[iter] = texture2D(cur_depth_map, newUV).x;
		previousDepths[iter] = texture2D(prev_depth_map, newUV + closestVec).x;
	}

	outColor = CustomResolve(previousDepths, currentDepths, closestVec);
}

/**
*	TODO: PREV_DEPTH_MAP IS OPTIMIZED OUT! WHY? WHYYYYYYYYYYYYYYYYYYYYYYYY?
*/
