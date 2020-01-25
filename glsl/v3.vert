#version 130

uniform mat4 screenTransform;

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 positionF;
out vec3 colorF;
out vec3 normalF;

void main()
{
	positionF = position;
	colorF = color;
	normalF = normal;
	gl_Position = screenTransform * vec4(position, 1);
}
