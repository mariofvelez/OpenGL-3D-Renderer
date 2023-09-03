#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <gl_util/Shader.h>
#include <gl_util/Camera.h>

#include "Model.h"
#include "Light.h"

#include <vector>
#include <ctime>

struct RenderObject
{
	unsigned int VAO;
	unsigned int texture;
	unsigned int num_elements;

	glm::mat4 model;

	RenderObject(unsigned int VAO, unsigned int texture, unsigned int num_elements, glm::mat4 model) :
		VAO(VAO), texture(texture), num_elements(num_elements), model(model) {}

	void setModelUniform(Shader* shader)
	{
		model_uniform_loc = shader->uniformLoc("model");
		glUniformMatrix4fv(model_uniform_loc, 1, GL_FALSE, glm::value_ptr(model));
	}

private:
	unsigned int model_uniform_loc = 9999;
};
class Renderer
{
	Camera* m_camera;

	GLFWwindow* window;

	int m_screen_width;
	int m_screen_height;

	glm::mat4* curr_view;
	glm::mat4* curr_projection;
	unsigned int m_ubo_matrices;

	std::vector<RenderObject> m_render_objects;

	std::vector<Model*> m_models;
	std::vector<glm::mat4*> m_model_transforms;

	unsigned int m_cubemap_VAO;
	unsigned int m_light_VAO;

	unsigned int m_texture;
	unsigned int m_cubemap_texture;

	DirLight* m_dirlight;
	PointLight* m_pointlight;
	SpotLight* m_spotlight;

	const float m_shadow_r = 10.0f;
	const float m_shadow_near_plane = -20.0f;
	const float m_shadow_far_plane = 20.0f;

	Shader* m_shader;
	Shader* m_outline_shader;
	Shader* m_light_shader;
	Shader* m_skybox_shader;
	Shader* m_shadow_shader;
	Shader* m_point_shadow_shader;

	unsigned int m_model_loc;
	unsigned int m_outline_model_loc;
	unsigned int m_light_model_loc;
	unsigned int m_shadow_model_loc;
	unsigned int m_point_shadow_model_loc;

	unsigned int m_shadow_matrix_loc_shadow;
	unsigned int m_shadow_matrix_loc_point_shadow;

	unsigned int m_point_shadow_lightpos_loc;
	unsigned int m_point_shadow_farplane_loc;

public:
	Renderer(GLFWwindow* window) : window(window)
	{
		m_shader = new Shader("shaders/Vertex.shader", "shaders/Fragment.shader");
		m_outline_shader = new Shader("shaders/Vertex.shader", "shaders/OutlineFragment.shader");
		m_light_shader = new Shader("shaders/LightVertex.shader", "shaders/LightFragment.shader");
		m_skybox_shader = new Shader("shaders/SkyboxVertex.shader", "shaders/SkyboxFragment.shader");
		m_shadow_shader = new Shader("shaders/ShadowVertex.shader", "shaders/ShadowFragment.shader");
		m_point_shadow_shader = new Shader("shaders/PointLightShadowVertex.shader", "shaders/PointLightShadowFragment.shader");
		m_point_shadow_shader->addGeometryShader("shaders/PointLightShadowGeometry.shader");

		glfwGetWindowSize(window, &m_screen_width, &m_screen_height);

		m_render_objects.reserve(4);

		m_models.reserve(20);
		m_model_transforms.reserve(20);

		m_model_loc = m_shader->uniformLoc("model");
		m_outline_model_loc = m_outline_shader->uniformLoc("model");
		m_light_model_loc = m_light_shader->uniformLoc("model");
		m_shadow_model_loc = m_shadow_shader->uniformLoc("model");
		m_point_shadow_model_loc = m_point_shadow_shader->uniformLoc("model");

		m_shadow_matrix_loc_shadow = m_shadow_shader->uniformLoc("shadowSpaceMatrix");
		m_shadow_matrix_loc_point_shadow = m_point_shadow_shader->uniformLoc("shadow_matrices");

		m_point_shadow_lightpos_loc = m_point_shadow_shader->uniformLoc("light_pos");
		m_point_shadow_farplane_loc = m_point_shadow_shader->uniformLoc("far_plane");

		// skybox
		std::vector<std::string> faces
		{
			"right.jpg",
			"left.jpg",
			"top.jpg",
			"bottom.jpg",
			"front.jpg",
			"back.jpg"
		};
		m_cubemap_texture = CubemapFromFile(faces, "skybox", true);

		float cubemapVertices[] = {
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		unsigned int cubemapVBO;
		glGenBuffers(1, &cubemapVBO);

		glGenVertexArrays(1, &m_cubemap_VAO);

		glBindVertexArray(m_cubemap_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), cubemapVertices, GL_STATIC_DRAW);

		// vertex positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// uniform buffer objects
		unsigned int uniform_block_index_vertex = glGetUniformBlockIndex(m_shader->m_ID, "Matrices"); // optional, could use binding = 0 in shader
		unsigned int uniform_block_index_light = glGetUniformBlockIndex(m_light_shader->m_ID, "Matrices");
		unsigned int uniform_block_index_skybox = glGetUniformBlockIndex(m_skybox_shader->m_ID, "Matrices");

		glUniformBlockBinding(m_shader->m_ID, uniform_block_index_vertex, 0);
		glUniformBlockBinding(m_light_shader->m_ID, uniform_block_index_light, 0);
		glUniformBlockBinding(m_skybox_shader->m_ID, uniform_block_index_skybox, 0);

		glGenBuffers(1, &m_ubo_matrices);

		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_matrices);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_matrices, 0, 2 * sizeof(glm::mat4));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}
	~Renderer()
	{
		delete(m_shader);
		delete(m_outline_shader);
		delete(m_light_shader);
		delete(m_skybox_shader);
		delete(m_shadow_shader);
	}
	
	void addRenderObject(unsigned int VAO, unsigned int texture, unsigned int num_elements, glm::mat4& model)
	{
		m_render_objects.emplace_back(VAO, texture, num_elements, model);
	}

	void addModel(Model* model, glm::mat4* transform)
	{
		m_models.push_back(model);
		m_model_transforms.push_back(transform);
	}
	
	void setDirLight(DirLight* light)
	{
		m_dirlight = light;
	}

	void setPointLight(PointLight* light)
	{
		m_pointlight = light;
	}

	void setSpotLight(SpotLight* light)
	{
		m_spotlight = light;
	}
	
	void setTexture(unsigned int texture)
	{
		m_texture = texture;
	}

	void setLightVAO(unsigned int light_VAO)
	{
		m_light_VAO = light_VAO;
	}

	void setCurrView(glm::mat4* view, glm::mat4* proj)
	{
		curr_view = view;
		curr_projection = proj;
	}

	void setCamera(Camera* camera)
	{
		m_camera = camera;
	}

	void updateLightUniforms()
	{
		m_shader->use();

		m_dirlight->uniformShader(m_shader, "dirlight");
		m_pointlight->uniformShader(m_shader, "pointlight");
		m_spotlight->uniformShader(m_shader, "spotlight");
	}

	void updateUniformBuffer(glm::mat4& view, glm::mat4& proj)
	{
		curr_view = &view;
		curr_projection = &proj;

		// update view and projection uniform buffer
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_matrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(proj));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void draw()
	{
		// set viewPos uniform for lighting
		m_shader->use();
		m_shader->setVec3("viewPos", m_camera->m_Pos);

		if (m_dirlight->casts_shadow)
		{
			m_shader->setInt("dirlight.shadow_map", 4);
			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, m_dirlight->shadow_map);

			glm::mat4 shadow_proj = glm::ortho(-m_shadow_r, m_shadow_r, -m_shadow_r, m_shadow_r, m_shadow_near_plane, m_shadow_far_plane);

			glm::vec3 eye = glm::vec3(m_dirlight->position);
			glm::vec3 target = eye + m_dirlight->direction;
			glm::mat4 shadow_view = glm::lookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::mat4 shadow_space_matrix = shadow_proj * shadow_view;

			unsigned int shadow_space_matrix_loc = m_shader->uniformLoc("shadow_projection");
			glUniformMatrix4fv(shadow_space_matrix_loc, 1, GL_FALSE, glm::value_ptr(shadow_space_matrix));
		}

		if (m_pointlight->casts_shadow)
		{
			m_shader->setInt("pointlight.shadow_map", 5);
			glActiveTexture(GL_TEXTURE0 + 5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_pointlight->shadow_map);

			//float near = 0.1f;
			//float far = 25.0f;
			//glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), 1.0f, near, far);

			//std::vector<glm::mat4> shadow_transforms;
			//shadow_transforms.reserve(6);
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)));
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)));
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)));
			//shadow_transforms.push_back(shadow_proj * glm::lookAt(m_pointlight->position, m_pointlight->position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

			////////glUniformMatrix4fv(glGetUniformLocation(m_point_shadow_shader->m_ID, "shadow_matrices"), 6, GL_FALSE, glm::value_ptr(shadow_transforms[0]));

			//for (unsigned int i = 0; i < 6; ++i)
			//{
			//	std::string name = "shadow_matrices[" + std::to_string(i) + "]";
			//	unsigned int loc = glGetUniformLocation(m_point_shadow_shader->m_ID, name.c_str());

			//	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(shadow_transforms[i]));
			//}
		}

		if (m_spotlight->casts_shadow)
		{
			m_shader->setInt("spotlight.shadow_map", 6);
			glActiveTexture(GL_TEXTURE0 + 6);
			glBindTexture(GL_TEXTURE_2D, m_spotlight->shadow_map);
		}
		

		// skybox rendering
		glDisable(GL_DEPTH_TEST);
		{
			m_skybox_shader->use();

			glBindVertexArray(m_cubemap_VAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture); //m_cubemap_texture
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glEnable(GL_DEPTH_TEST);

		m_shader->use();
		// render all VAOs
		glActiveTexture(GL_TEXTURE0);
		for (unsigned int i = 0; i < m_render_objects.size(); ++i)
		{
			RenderObject ro = m_render_objects[i];
			ro.setModelUniform(m_shader);
			glBindVertexArray(ro.VAO);

			glBindTexture(GL_TEXTURE_2D, ro.texture);

			glDrawElements(GL_TRIANGLES, ro.num_elements, GL_UNSIGNED_INT, 0);
		}

		// render all models
		for (unsigned int i = 0; i < m_models.size(); ++i)
		{
			glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(*m_model_transforms[i]));

			m_models[i]->Draw(*m_shader);
		}

		// render all lights
		m_light_shader->use();
		glBindVertexArray(m_light_VAO);
		{
			m_light_shader->setVec3("lightColor", m_pointlight->color);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, m_pointlight->position);

			glUniformMatrix4fv(m_light_model_loc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		{
			m_light_shader->setVec3("lightColor", m_spotlight->color);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, m_spotlight->position);

			glUniformMatrix4fv(m_light_model_loc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
	}

	void drawDirectionalShadow()
	{
		m_shadow_shader->use();

		glm::mat4 shadow_proj = glm::ortho(-m_shadow_r, m_shadow_r, -m_shadow_r, m_shadow_r, m_shadow_near_plane, m_shadow_far_plane);

		glm::vec3 eye = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 target = eye + m_dirlight->direction;
		glm::mat4 shadow_view = glm::lookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 shadow_space_matrix = shadow_proj * shadow_view;

		glUniformMatrix4fv(m_shadow_matrix_loc_shadow, 1, GL_FALSE, glm::value_ptr(shadow_space_matrix));

		// render all VAOs
		for (unsigned int i = 0; i < m_render_objects.size(); ++i)
		{
			RenderObject ro = m_render_objects[i];
			glUniformMatrix4fv(m_shadow_model_loc, 1, GL_FALSE, glm::value_ptr(ro.model));
			glBindVertexArray(ro.VAO);

			glDrawElements(GL_TRIANGLES, ro.num_elements, GL_UNSIGNED_INT, 0);
		}

		// render all models
		for (unsigned int i = 0; i < m_models.size(); ++i)
		{
			glUniformMatrix4fv(m_shadow_model_loc, 1, GL_FALSE, glm::value_ptr(*m_model_transforms[i]));

			m_models[i]->Draw(*m_shader);
		}
	}

	void drawPointShadow()
	{
		m_point_shadow_shader->use();

		float near = 0.1f;
		float far = 25.0f;
		glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), 1.0f, near, far);

		std::vector<glm::mat4> shadow_transforms;
		shadow_transforms.reserve(6);
		glm::vec3 pos = m_pointlight->position;
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadow_transforms.push_back(shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		for (unsigned int i = 0; i < 6; ++i)
		{
			std::string name = "shadow_matrices[" + std::to_string(i) + "]";

			//m_point_shadow_shader->setMat4(name, shadow_transforms[i]);
			glUniformMatrix4fv(m_shadow_matrix_loc_point_shadow + i, 1, GL_FALSE, glm::value_ptr(shadow_transforms[i]));
		}

		glUniform3f(m_point_shadow_lightpos_loc, pos.x, pos.y, pos.z);

		glUniform1f(m_point_shadow_farplane_loc, 25.0f);

		// render all VAOs
		for (unsigned int i = 0; i < m_render_objects.size(); ++i)
		{
			RenderObject ro = m_render_objects[i];
			glUniformMatrix4fv(m_point_shadow_model_loc, 1, GL_FALSE, glm::value_ptr(ro.model));
			glBindVertexArray(ro.VAO);

			glDrawElements(GL_TRIANGLES, ro.num_elements, GL_UNSIGNED_INT, 0);
		}

		// render all models
		for (unsigned int i = 0; i < m_models.size(); ++i)
		{
			glUniformMatrix4fv(m_point_shadow_model_loc, 1, GL_FALSE, glm::value_ptr(*m_model_transforms[i]));

			m_models[i]->Draw(*m_shader);
		}
	}
	int shadow_renders = 0;
	void drawShadows()
	{
		if (m_dirlight->casts_shadow)
		{
			glCullFace(GL_FRONT);
			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, DirLight::shadow_width, DirLight::shadow_height);
			glBindFramebuffer(GL_FRAMEBUFFER, m_dirlight->shadow_fbo);
			glClear(GL_DEPTH_BUFFER_BIT);

			drawDirectionalShadow();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		if (m_pointlight->casts_shadow)
		{
			glCullFace(GL_FRONT);
			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, 1024, 1024);
			glBindFramebuffer(GL_FRAMEBUFFER, m_pointlight->shadow_fbo);
			glClear(GL_DEPTH_BUFFER_BIT);

			drawPointShadow();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void render()
	{
		// shadow casts
		//drawShadows();
	}
};