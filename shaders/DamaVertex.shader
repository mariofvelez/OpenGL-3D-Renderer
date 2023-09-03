#version 330 core
layout(location = 0) in aPos;
layout(location = 1) in aNormal;
layout(location = 2) in aTexCoord;

out vec2 TexCoord;
out vec3 normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	TexCoord = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal; // for non-uniform scaling
}