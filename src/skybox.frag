#version 410

uniform samplerCube cubeTex;

in vec3 texCoords;
out vec4 fragColor;

void main()
{
	fragColor = texture(cubeTex, texCoords);
}