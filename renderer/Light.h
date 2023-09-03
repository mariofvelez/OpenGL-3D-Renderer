#pragma once

#include <glm/glm.hpp>

#include <gl_util/Shader.h>

#include "Util.h"

#include <string>

enum LightType
{
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT
};
struct Light
{
	LightType type;

	glm::vec3 color;

	glm::vec3 position;
	glm::vec3 direction;

	float ambient_strength;

	unsigned int shadow_fbo;
	unsigned int shadow_map;

	const bool casts_shadow;

	Light(glm::vec3 color, glm::vec3 position, glm::vec3 direction, float ambient_strength, bool casts_shadow) : color(color), position(position), direction(direction), ambient_strength(ambient_strength), casts_shadow(casts_shadow) {};

	void setColor(float r, float g, float b)
	{
		color.x = r;
		color.y = g;
		color.z = b;
	}
	void uniformShader(Shader* shader, const std::string& uniform_name);

	// gets the unifrom locations needed for this light from the shader
	void setUniformLocations(Shader* shader);
};
struct DirLight : public Light
{
	const static unsigned int shadow_width = 2048;
	const static unsigned int shadow_height = 2048;

	DirLight(glm::vec3 color, glm::vec3 position, glm::vec3 direction, float ambient_strength, bool casts_shadow)
		: Light(color, position, direction, ambient_strength, casts_shadow)
	{
		type = DIRECTIONAL_LIGHT;

		if (casts_shadow)
		{
			shadow_fbo = createDirectionalDepthMapBuffer(&shadow_map, shadow_width, shadow_height);
		}
	}
	~DirLight()
	{
		if(casts_shadow)
			glDeleteFramebuffers(1, &shadow_fbo);
	}
	void uniformShader(Shader* shader, const std::string& uniform_name)
	{
		shader->setVec3(uniform_name + ".direction", direction);

		shader->setVec3(uniform_name + ".ambient", color.x * ambient_strength, color.y * ambient_strength, color.z * ambient_strength);
		shader->setVec3(uniform_name + ".diffuse", color.x, color.y, color.z);
		shader->setVec3(uniform_name + ".specular", color.x, color.y, color.z);

		shader->setBool(uniform_name + ".casts_shadow", casts_shadow);
	}
};
struct PointLight : public Light
{
	float constant;
	float linear;
	float quadratic;

	PointLight(glm::vec3 color, glm::vec3 position, float ambient_strength, bool casts_shadow)
		: Light(color, position, glm::vec3(0.0f, 0.0f, 0.0f), ambient_strength, casts_shadow), constant(1.0f), linear(0.09f), quadratic(0.032f)
	{
		type = POINT_LIGHT;

		if (casts_shadow)
		{
			shadow_fbo = createOmnidirectionalDepthMapBuffer(&shadow_map, 1024);
		}
	}
	~PointLight()
	{
		if (casts_shadow)
		{
			glDeleteFramebuffers(1, &shadow_fbo);
		}
	}
	void uniformShader(Shader* shader, const std::string& uniform_name)
	{
		shader->setVec3(uniform_name + ".position", position);

		shader->setVec3(uniform_name + ".ambient", color.x * ambient_strength, color.y * ambient_strength, color.z * ambient_strength);
		shader->setVec3(uniform_name + ".diffuse", color.x, color.y, color.z);
		shader->setVec3(uniform_name + ".specular", color.x, color.y, color.z);

		shader->setFloat(uniform_name + ".constant", constant);
		shader->setFloat(uniform_name + ".linear", linear);
		shader->setFloat(uniform_name + ".quadratic", quadratic);

		shader->setBool(uniform_name + ".casts_shadow", casts_shadow);
	}
};
struct SpotLight : Light
{
	float inner_cutoff;
	float outer_cutoff;

	float constant;
	float linear;
	float quadratic;

	SpotLight(glm::vec3 color, glm::vec3 position, glm::vec3 direction, float ambient_strength, bool casts_shadow)
		: Light(color, position, direction, ambient_strength, casts_shadow), inner_cutoff(glm::cos(glm::radians(12.5f))), outer_cutoff(glm::cos(glm::radians(17.5f))),
		constant(1.0f), linear(0.09f), quadratic(0.032f)
	{
		type = SPOT_LIGHT;
	}

	void uniformShader(Shader* shader, const std::string& uniform_name)
	{
		shader->setVec3(uniform_name + ".position", position);
		shader->setVec3(uniform_name + ".direction", direction);

		shader->setFloat(uniform_name + ".innercutoff", inner_cutoff);
		shader->setFloat(uniform_name + ".outercutoff", outer_cutoff);

		shader->setVec3(uniform_name + ".ambient", color.x * ambient_strength, color.y * ambient_strength, color.z * ambient_strength);
		shader->setVec3(uniform_name + ".diffuse", color.x, color.y, color.z);
		shader->setVec3(uniform_name + ".specular", color.x, color.y, color.z);

		shader->setFloat(uniform_name + ".constant", constant);
		shader->setFloat(uniform_name + ".linear", linear);
		shader->setFloat(uniform_name + ".quadratic", quadratic);
	}
};