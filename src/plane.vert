#version 410

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in float height;

uniform mat4 view, proj;

out vec3 posEye;
out vec2 coords;
out float heightOut;

void main()
{
	coords = texCoords; //repeat texture over plane
	gl_Position = proj * view * vec4(vertexPos, 1.0);

	heightOut = height;

	posEye = (view * vec4(vertexPos, 1.0)).xyz;
}