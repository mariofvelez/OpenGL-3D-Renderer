#version 420 core
in vec3 TexCoord;

out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
	FragColor = texture(skybox, TexCoord);

	//rendering point shadow map
	/*float closest_depth = texture(skybox, TexCoord).r;
	FragColor = vec4(vec3(closest_depth), 1.0);*/
}