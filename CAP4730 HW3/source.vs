#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 m;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
void main()
{
	gl_Position = projection * view * m * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	ourColor = aColor;
}