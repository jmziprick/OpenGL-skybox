#version 410

in vec3 posEye;
in vec2 coords;
out vec4 fragColor;

in float heightOut;

uniform sampler2D tex;

//fog
const vec3 fogColor = vec3(0.2, 0.2, 0.2);
const float minFogRad = 300;
const float maxFogRad = 900;

void main()
{
	vec4 texture = texture2D(tex, coords);
	fragColor = texture;

	float distance = length(-posEye);
	float fogFactor = (distance - minFogRad) / (maxFogRad - minFogRad);
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	float height = heightOut;

	if(height > 45)
		fragColor.r *= height / 45;

	if(height > 16)
		height = 16;

	if(height < 10)
		height = 10;

	fragColor.rgb = mix(fragColor.rgb, fogColor, fogFactor);
	fragColor *= height / 15;
}