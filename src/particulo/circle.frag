#version 450 core
precision highp float;
out vec4 FragColor;
in vec4 coord;
in vec4 col;

float aastep(float threshold, float value) {
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main()
{
	if(col.a == 0.0)
	{
		discard;
	}
	FragColor.rgb = col.rgb;
	// FragColor.rgb = vec3(0.424, 0.361, 0.906);
	
	FragColor.a = aastep(-1.0, -1.0 * length(coord.xyz));
	FragColor.a *= col.a;
	
	// FragColor.a += 0.2;
	if(FragColor.a == 0.0)
	{
		discard;
	}
	// else
	// {
	// 	// FragColor.a /= 1.25;
	// }
}