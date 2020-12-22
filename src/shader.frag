#version 410

in vec3 posEye;
in float dist;
uniform float color;

//fog
const vec3 fogColor = vec3(0.2, 0.2, 0.2);
const float minFogRad = 300;
const float maxFogRad = 900;

void main()
{
	vec4 tex = vec4(1.0, 0, 0, 0);
	tex.xyz *= dist;

	float distance = length(-posEye);
	float fogFactor = (distance - minFogRad) / (maxFogRad - minFogRad);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	
	gl_FragColor = tex;
	//gl_FragColor.rgb = mix(gl_FragColor.rgb, fogColor, fogFactor);
}