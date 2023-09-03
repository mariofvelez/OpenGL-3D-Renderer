#version 420 core
in vec4 FragPos;

uniform vec3 light_pos;
uniform float far_plane;

void main()
{
	float light_distance = length(FragPos.xyz - light_pos);

	light_distance = light_distance / far_plane;

	gl_FragDepth = light_distance;
}