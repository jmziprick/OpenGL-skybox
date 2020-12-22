#version 410

layout(location = 0) in vec3 vertexPos;

uniform mat4 view, proj;

out vec3 texCoords;

void main()
{
	texCoords = vertexPos;
	gl_Position = proj * view * vec4(vertexPos, 1.0);
}