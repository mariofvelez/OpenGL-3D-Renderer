#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 instanceMatrix;

//out vec3 vertexColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 LocalPos;

uniform float angle;

layout(std140, binding = 0) uniform Matrices
{
	mat4 view;
	mat4 projection;
};

void main()
{
	vec3 pos = aPos;
	pos.x += sin(angle * 8 + pos.z * 3 - pos.y) * (-pos.z * 0.5 + 1.5) * 0.1;
	vec4 pmodel = instanceMatrix * vec4(pos, 1.0);
	gl_Position = projection * view * pmodel;
	FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
	
	TexCoord = aTexCoord;
	Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal; // for non-uniform scaling
	LocalPos = aPos;
}