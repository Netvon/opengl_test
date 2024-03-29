#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;

uniform mat4 origin;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 ourColor;
out vec3 normal;
out vec3 pos;

void main()
{
	pos = vec3(model * vec4(aPos, 1.0));
	ourColor = aColor;
	normal = mat3(transpose(inverse(model))) * aNormal;
	//normal = vec3(transpose(inverse(model)) * vec4(aPos, 1.0));

	gl_Position = projection * view * vec4(pos, 1.0);
}