#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
//layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexCoord;

//out vec3 vertexColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 LocalPos;
out vec4 FragPosLightSpace;

uniform float angle;
uniform mat4 model;
uniform mat4 shadow_projection;

layout(std140, binding = 0) uniform Matrices
{
	mat4 view;
	mat4 projection;
};

void main()
{
	vec3 pos = aPos;
	vec4 pmodel = model * vec4(pos, 1.0);
	//pmodel.x += sin(-angle * 8 + pmodel.x * 3)* (pmodel.x * 0.5 + 0.5) * 0.2;
	gl_Position = projection * view * pmodel;// vec4(len * cos(theta + angle), len * sin(theta + angle), aPos.z, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	//vertexColor = aCol;
	TexCoord = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal; // for non-uniform scaling
	LocalPos = aPos;
	//Normal = mat3(model) * aNormal;
	FragPosLightSpace = shadow_projection * vec4(FragPos, 1.0);
}