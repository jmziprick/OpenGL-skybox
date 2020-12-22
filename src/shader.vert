#version 410

layout(location = 0) in vec3 vertexPos;

uniform mat4 view, proj;
out float dist;
out vec3 posEye;

void main()
{
	gl_Position = proj * view * vec4(vertexPos * 10 + vec3(1000.0f, 500.0f, 1000.0f) + vec3(0, -400, 0), 1.0);
	dist = vertexPos.z;
	posEye = (view * vec4(vertexPos, 1.0)).xyz;
}