#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>

#include <string>
#include <vector>

#include "stb_image.h"

#define print(x) std::cout << x << std::endl

unsigned int TextureFromFile(const char* path, const std::string& directory, bool linearize, unsigned int texture_type = GL_TEXTURE_2D)
{
	stbi_set_flip_vertically_on_load(true);

	std::string filename = std::string(path);
	if (directory != "")
		filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, num_components;
	print("filename: " << filename);
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
	print("stbi loaded texture: " << textureID);
	if (data)
	{

		GLenum source_format;
		GLenum format;
		if (num_components == 1)
		{
			source_format = GL_RED;
			format = GL_RED;
		}
		else if (num_components == 3)
		{
			if (linearize)
				source_format = GL_SRGB;
			else
				source_format = GL_RGB;
			format = GL_RGB;
		}
		else if (num_components == 4)
		{
			if (linearize)
				source_format = GL_SRGB_ALPHA;
			else
				source_format = GL_RGBA;
			format = GL_RGBA;
		}

		glBindTexture(texture_type, textureID);
		glTexImage2D(texture_type, 0, source_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(texture_type);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}
unsigned int CubemapFromFile(std::vector<std::string>& faces, const std::string& directory, bool linearize)
{
	stbi_set_flip_vertically_on_load(false);

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, num_channels;

	for (unsigned int i = 0; i < faces.size(); ++i)
	{
		std::string filename = faces[i];
		if (directory != "")
			filename = directory + '/' + filename;

		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_channels, 0);

		if (data)
		{
			GLenum source_format;
			GLenum format;
			if (num_channels == 1)
			{
				source_format = GL_RED;
				format = GL_RED;
			}
			else if (num_channels == 3)
			{
				if (linearize)
					source_format = GL_SRGB;
				else
					source_format = GL_RGB;
				format = GL_RGB;
			}
			else if (num_channels == 4)
			{
				if (linearize)
					source_format = GL_SRGB_ALPHA;
				else
					source_format = GL_RGBA;
				format = GL_RGB;
			}

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, source_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << filename << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
/**
vertices size should be 8 * (width+1) * (height+1)
indices size should be 6 * width * height
*/
void createSphere(int width, int height, float rho, float* vertices, int* indices)
{
	int vertex_index = 0;
	int indices_index = 0;
	for (int h = 0; h <= height; ++h)
	{
		for (int w = 0; w <= width; ++w)
		{
			float u = ((float)w / (float)width);
			float v = ((float)h / (float)height);
			float theta = u * glm::two_pi<float>();
			float phi = v * glm::pi<float>();

			float x = glm::sin(phi) * glm::cos(theta);
			float z = glm::sin(phi) * glm::sin(theta);
			float y = glm::cos(phi);

			vertices[vertex_index * 8 + 0] = x * rho;
			vertices[vertex_index * 8 + 1] = -y * rho;
			vertices[vertex_index * 8 + 2] = z * rho;
			vertices[vertex_index * 8 + 3] = x;
			vertices[vertex_index * 8 + 4] = -y;
			vertices[vertex_index * 8 + 5] = z;
			vertices[vertex_index * 8 + 6] = u;
			vertices[vertex_index * 8 + 7] = v;

			//if (h < height || w < width)
			//{
			//	int top_left  = vertex_index;
			//	int top_right = vertex_index + 1;
			//	int bot_left  = vertex_index + width + 1;
			//	int bot_right = vertex_index + width + 2;
			//	indices[indices_index * 6 + 0] = top_left;
			//	indices[indices_index * 6 + 1] = bot_right;
			//	indices[indices_index * 6 + 2] = top_right;
			//	indices[indices_index * 6 + 3] = bot_right;
			//	indices[indices_index * 6 + 4] = top_left;
			//	indices[indices_index * 6 + 5] = bot_left;

			//	++indices_index;
			//}

			++vertex_index;
		}
	}
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			int top_left = w * (width + 1) + h;
			int top_right = top_left + 1;
			int bot_left = top_left + width + 1;
			int bot_right = top_right + width + 1;
			indices[indices_index * 6 + 0] = top_left;
			indices[indices_index * 6 + 1] = bot_right;
			indices[indices_index * 6 + 2] = top_right;
			indices[indices_index * 6 + 3] = bot_right;
			indices[indices_index * 6 + 4] = top_left;
			indices[indices_index * 6 + 5] = bot_left;

			++indices_index;
		}
	}
}
float randFloat(float min, float max)
{
	float r = (float)(rand()) / (float)(RAND_MAX);
	return min + (max - min) * r;
}
unsigned int createFrameBuffer(bool multi_sample, unsigned int* frame_texture, unsigned int width, unsigned int height)
{
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, frame_texture);
	if (multi_sample)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *frame_texture);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);

		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, *frame_texture, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, *frame_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *frame_texture, 0);
	}

	if (multi_sample)
	{
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		if (multi_sample)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}
unsigned int createDirectionalDepthMapBuffer(unsigned int* depth_map_texture, unsigned int width, unsigned int height)
{
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);

	glGenTextures(1, depth_map_texture);
	glBindTexture(GL_TEXTURE_2D, *depth_map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_map_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Depth Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}
unsigned int createOmnidirectionalDepthMapBuffer(unsigned int* depth_cubemap_texture, unsigned int size)
{
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);

	glGenTextures(1, depth_cubemap_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *depth_cubemap_texture);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depth_cubemap_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	print("framebuffer errors: " << glGetError());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Depth Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}
int MAX(int a, int b)
{
	return a < b ? b : a;
}
int MIN(int a, int b)
{
	return a < b ? a : b;
}
bool glfwSetWindowCenter(GLFWwindow* window)
{
	if (!window)
		return false;

	int sx = 0, sy = 0;
	int px = 0, py = 0;
	int mx = 0, my = 0;
	int monitor_count = 0;
	int best_area = 0;
	int final_x = 0, final_y = 0;

	glfwGetWindowSize(window, &sx, &sy);
	glfwGetWindowPos(window, &px, &py);

	// Iterate throug all monitors
	GLFWmonitor** m = glfwGetMonitors(&monitor_count);
	if (!m)
		return false;

	for (int j = 0; j < monitor_count; ++j)
	{

		glfwGetMonitorPos(m[j], &mx, &my);
		const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
		if (!mode)
			continue;

		// Get intersection of two rectangles - screen and window
		int minX = MAX(mx, px);
		int minY = MAX(my, py);

		int maxX = MIN(mx + mode->width, px + sx);
		int maxY = MIN(my + mode->height, py + sy);

		// Calculate area of the intersection
		int area = MAX(maxX - minX, 0) * MAX(maxY - minY, 0);

		// If its bigger than actual (window covers more space on this monitor)
		if (area > best_area)
		{
			// Calculate proper position in this monitor
			final_x = mx + (mode->width - sx) / 2;
			final_y = my + (mode->height - sy) / 2;

			best_area = area;
		}

	}

	// We found something
	if (best_area)
		glfwSetWindowPos(window, final_x, final_y);

	// Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
	else
	{
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		if (primary)
		{
			const GLFWvidmode* desktop = glfwGetVideoMode(primary);

			if (desktop)
				glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
			else
				return false;
		}
		else
			return false;
	}

	return true;
}