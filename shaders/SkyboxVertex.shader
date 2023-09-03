#version 420 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoord;

layout(std140, binding = 0) uniform Matrices
{
	mat4 view;
	mat4 projection;
};

void main()
{
	TexCoord = aPos;
	mat4 v = mat4(mat3(view));
	gl_Position = projection * v * vec4(aPos, 1.0);
}