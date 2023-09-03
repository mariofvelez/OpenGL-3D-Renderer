#version 420 core
layout(location = 0) in vec3 aPos;

uniform mat4 shadowSpaceMatrix;
uniform mat4 model;

void main()
{
	gl_Position = shadowSpaceMatrix * model * vec4(aPos, 1.0);
}