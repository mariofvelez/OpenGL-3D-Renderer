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

#include "renderer/Model.h"
#include "renderer/Util.h"
#include "renderer/Light.h"
#include "renderer/Renderer.h"

#include "stb_image.h"

#include <vector>
#include <ctime>

#define DEBUG_LOG
#define DEBUG_TIME

#ifdef DEBUG_LOG
	#define print(x) std::cout << x << std::endl
#endif
#ifndef DEBUG_LOG
	#define print(x)
#endif
#ifdef DEBUG_TIME
	#define calc_time(x) std::time_t t1 = std::time(nullptr); x; std::time_t t2 = std::time(nullptr); std::cout << "time: " << (t2 - t1) << std::endl
#endif
#ifndef DEBUG_TIME
	#define calc_time(x)
#endif

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f),
			  glm::vec3(0.0f, 0.0f, -1.0f),
			  glm::vec3(0.0f, 1.0f, 0.0f));



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	static float last_X = 450;
	static float last_Y = 300;

	static bool firstMouse = true;
	if (firstMouse)
	{
		last_X = xpos;
		last_Y = ypos;
		firstMouse = false;
	}

	float offsetX = xpos - last_X;
	float offsetY = last_Y - ypos;

	last_X = xpos;
	last_Y = ypos;

	static float sensitivity = 0.001f;
	offsetX *= sensitivity;
	offsetY *= sensitivity;

	camera.onMouseCallback(offsetX, offsetY);
}
void scrollCallback(GLFWwindow* window, double offsetX, double offsetY)
{
	camera.onScrollCallback(offsetX, offsetY);
}
void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	camera.processCameraInput(window, deltaTime);
}

int main()
{
	// creating the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	unsigned int screen_width = 1600;
	unsigned int screen_height = 900;

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, screen_width, screen_height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowCenter(window);


	// creating the renderer
	Renderer* renderer = new Renderer(window);

	renderer->setCamera(&camera);


	// generating textures
	unsigned int texture = TextureFromFile("grass_texture1.png", "", false);
	unsigned int texture2 = TextureFromFile("box.jpg", "", true);
	unsigned int texture3 = TextureFromFile("plant.jpg", "", true);
	renderer->setTexture(texture);
	
	// texture wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	// border color only for GL_CLAMP_TO_BORDER option
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// texel filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// print number of vertex attributes supported
	int nAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes);
	std::cout << "Max vertex attributes supported: " << nAttributes << std::endl;

	// cubes
	float vertices[] = {
	 0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

	 0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f
	};
	unsigned int indices[36];
	for (int i = 0; i < 36; ++i)
		indices[i] = i;

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// VAO stores attributes
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copies vertices data into VBO

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// vertex colors
	/*glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);*/

	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// vertex texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glm::vec3 cubePositions[] = {
		glm::vec3(2.0f,  -2.4f, 4.0f),
		glm::vec3(-5.5f, -3.4f, -2.5f),
		glm::vec3(-3.8f, -2.4f, -3.3f),
		glm::vec3(2.4f,  -3.4f, -3.5f),
		glm::vec3(-3.7f, -2.4f, -7.5f),
		glm::vec3(1.3f,  -3.4f, -2.5f),
		glm::vec3(4.5f,  -3.4f, -7.5f),
		glm::vec3(1.5f,  -3.0f, -1.5f),
		glm::vec3(-2.3f, -3.4f, -1.5f)
	};

	for (unsigned int i = 0; i < 9; ++i)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[i]);
		renderer->addRenderObject(VAO, texture2, 36, model);
	}
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, -2.0f));
		model = glm::scale(model, glm::vec3(20.0f, 0.2f, 20.0f));
		renderer->addRenderObject(VAO, texture, 36, model);
	}


	// creating the ball
	const int ball_width = 40;
	const int ball_height = 40;

	float ball_vertices[8 * (ball_width + 1) * (ball_height + 1)];
	int ball_indices[6 * ball_width * ball_height];

	createSphere(ball_width, ball_height, 1.0f, ball_vertices, ball_indices);

	unsigned int bVBO;
	glGenBuffers(1, &bVBO);

	unsigned int bVAO;
	glGenVertexArrays(1, &bVAO);

	glBindVertexArray(bVAO);

	glBindBuffer(GL_ARRAY_BUFFER, bVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ball_vertices), ball_vertices, GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// vertex normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// vertex texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int bEBO;
	glGenBuffers(1, &bEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ball_indices), ball_indices, GL_STATIC_DRAW);

	{
		glm::vec3 ball_pos = glm::vec3(0.0f, 0.0f, -3.0f);
		glm::mat4 ball_model = glm::mat4(1.0f);
		ball_model = glm::translate(ball_model, ball_pos);

		renderer->addRenderObject(bVAO, texture3, 6 * ball_width * ball_height, ball_model);
	}


	// creating test model
	/*Model ping_pong_model("ping_pong/pingpong_table.obj");
	glm::mat4 ping_pong_transform = glm::mat4(1.0f);
	ping_pong_transform = glm::translate(ping_pong_transform, glm::vec3(0.0f, -3.9f, 0.0f));
	renderer->addModel(&ping_pong_model, &ping_pong_transform);*/


	//light
	unsigned int lVAO;
	glGenVertexArrays(1, &lVAO);

	glBindVertexArray(lVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int lEBO;
	glGenBuffers(1, &lEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	renderer->setLightVAO(lVAO);


	// lights
	DirLight* dir_light = new DirLight(glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, -1.0f, 1.4f), 0.1f, false);

	SpotLight* spot_light = new SpotLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 4.0f, -4.0f), glm::vec3(0.2f, -1.0f, 0.3f), 0.0f, false);

	renderer->setDirLight(dir_light);
	renderer->setSpotLight(spot_light);

	// point lights
	const unsigned int num_point_lights = 4;
	std::vector<PointLight> point_lights;
	point_lights.reserve(num_point_lights);

	{
		glm::vec3 pointLightPositions[] = {
			glm::vec3(0.7f,  0.2f,  2.0f),
			glm::vec3(2.3f, -3.8f, -5.2f),
			glm::vec3(-4.0f,  2.0f, -12.0f),
			glm::vec3(-5.0f,  0.0f, 4.0f)
		};
		glm::vec3 pointLightColors[] = {
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		};
		for (int i = 0; i < 1; ++i)
		{
			std::string num = std::to_string(i);

			point_lights.emplace_back(pointLightColors[i], pointLightPositions[i], 0.1f, false);

			renderer->setPointLight(&point_lights[i]);
		}
	}

	renderer->updateLightUniforms();

	// material
	//shader.setInt("material.diffuse1", 0); // texture 0
	//shader.setVec3("material.specular1", 0.8f, 0.8f, 0.8f);
	//shader.setFloat("material.shininess", 33.0f);

	//unsigned int viewLoc = shader.uniformLoc("view");
	//unsigned int projectionLoc = shader.uniformLoc("projection");

	//unsigned int outlineViewLoc = outline_shader.uniformLoc("view");
	//unsigned int outlineProjectionLoc = outline_shader.uniformLoc("projection");
	//std::string filename = "Models/Backpack/backpack.obj";
	//std::string filename = "swamp/scene.gltf";
	
	

	// projection
	//glm::mat4 ortho = glm::ortho(0.0f, 900.0f, 0.0f, 600.0f, 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	glEnable(GL_MULTISAMPLE);

	float mix_vals[] = {
		0.0f,
		0.1f,
		0.2f,
		0.3f,
		0.4f,
		0.5f,
		0.6f,
		0.7f,
		0.8f,
		1.0f
	};

	// framebuffer
	unsigned int ms_frame_texture;
	unsigned int msFBO = createFrameBuffer(true, &ms_frame_texture, screen_width, screen_height);

	unsigned int inter_frame_texture;
	unsigned int interFBO = createFrameBuffer(false, &inter_frame_texture, screen_width, screen_height);

	// quad
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);

	unsigned int quadVAO;
	glGenVertexArrays(1, &quadVAO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW); // copies vertices data into VBO

	// vertex positions
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// vertex texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	Shader screen_shader("shaders/ScreenVertex.shader", "shaders/ScreenFragment.shader");

	//std::vector<std::string> faces
	//{
	//	"right.jpg",
	//	"left.jpg",
	//	"top.jpg",
	//	"bottom.jpg",
	//	"front.jpg",
	//	"back.jpg"
	//};
	//unsigned int cubemap_texture = CubemapFromFile(faces, "skybox", true);

	//float cubemapVertices[] = {
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	-1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f
	//};

	//unsigned int cubemapVBO;
	//glGenBuffers(1, &cubemapVBO);

	//unsigned int cubemapVAO;
	//glGenVertexArrays(1, &cubemapVAO);

	//glBindVertexArray(cubemapVAO);

	//glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), cubemapVertices, GL_STATIC_DRAW);

	//// vertex positions
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	//glEnableVertexAttribArray(0);

	//Shader skybox_shader("shaders/SkyboxVertex.shader", "shaders/SkyboxFragment.shader");

	//unsigned int skyboxViewLoc = skybox_shader.uniformLoc("view");
	//unsigned int skyboxProjLoc = skybox_shader.uniformLoc("projection");


	// instancing fish
	float circle_radius = 170.0f;
	float offset_scale = 130.0f;
	unsigned int num_fish = 10000;

	glm::mat4* fish_transforms;
	fish_transforms = new glm::mat4[num_fish];

	srand(time(0));

	for (unsigned int i = 0; i < num_fish; ++i)
	{
		float theta = (float)(i) * glm::pi<float>() * 2.0f / (float)num_fish;
		glm::vec3 offset = glm::vec3(randFloat(0.0f, offset_scale), randFloat(0.0f, offset_scale), randFloat(0.0f, offset_scale));
		offset += glm::vec3(glm::cos(theta) * circle_radius, 0.0f, glm::sin(theta) * circle_radius);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, offset);
		//model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -theta, glm::vec3(0.0f, 1.0f, 0.0f));

		fish_transforms[i] = model;
	}

	std::string filename = "coral_fish/pez_amarillo.obj";
	calc_time(Model fish_model(filename));
	glm::mat4 fish_transform = glm::mat4(1.0f);
	fish_transform = glm::translate(fish_transform, glm::vec3(-3.0f, -1.0f, 0.0f));
	renderer->addModel(&fish_model, &fish_transform);

	Shader fish_shader("shaders/InstanceVertex.shader", "shaders/Fragment.shader");

	fish_shader.use();

	dir_light->uniformShader(&fish_shader, "dirlight");

	spot_light->uniformShader(&fish_shader, "spotlight");

	for (unsigned int i = 0; i < 1; ++i)
	{
		std::string num = std::to_string(i);
		
		point_lights[i].uniformShader(&fish_shader, "pointlight");
	}

	unsigned int fish_model_buffer;
	glGenBuffers(1, &fish_model_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, fish_model_buffer);
	glBufferData(GL_ARRAY_BUFFER, num_fish * sizeof(glm::mat4), &fish_transforms[0], GL_STATIC_DRAW);

	for (int i = 0; i < fish_model.meshes.size(); ++i)
	{
		unsigned int meshVAO = fish_model.meshes[i].VAO;
		glBindVertexArray(meshVAO);
		std::size_t vec4size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(1 * vec4size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(2 * vec4size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(3 * vec4size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}


	// uniform buffer objects
	//unsigned int uniform_block_index_vertex = glGetUniformBlockIndex(shader.m_ID, "Matrices"); // optional, could use binding = 0 in shader
	//unsigned int uniform_block_index_light  = glGetUniformBlockIndex(lightShader.m_ID, "Matrices");
	//unsigned int uniform_block_index_skybox = glGetUniformBlockIndex(skybox_shader.m_ID, "Matrices");

	//glUniformBlockBinding(shader.m_ID, uniform_block_index_vertex, 0);
	//glUniformBlockBinding(lightShader.m_ID, uniform_block_index_light, 0);
	//glUniformBlockBinding(skybox_shader.m_ID, uniform_block_index_skybox, 0);

	//unsigned int uboMatrices;
	//glGenBuffers(1, &uboMatrices);

	//glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	//glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));


	//unsigned int cubemap_texture = CubemapFromFile();

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// render loop
	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//input
		processInput(window, camera, deltaTime);

		const float light_speed = 4.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			point_lights[0].position.x -= light_speed * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			point_lights[0].position.x += light_speed * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			point_lights[0].position.z -= light_speed * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			point_lights[0].position.z += light_speed * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
			point_lights[0].position.y -= light_speed * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
			point_lights[0].position.y += light_speed * deltaTime;

		float time_val = glfwGetTime();
		/*glm::vec3 dir = glm::vec3(glm::cos(time_val), -1.0f, glm::sin(time_val));
		dir_light->direction = dir;*/
		renderer->updateLightUniforms();

		renderer->drawShadows();


		//rendering commands here
		glCullFace(GL_BACK);
		glViewport(0, 0, screen_width, screen_height);
		glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
		glm::vec3 clear_col = glm::vec3(204, 204, 204);
		clear_col /= 255.0f;
		glClearColor(clear_col.x, clear_col.y, clear_col.z, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		

		// view and projection
		//const float radius = 5.0f;
		//float cam_x = cos(time_val / 4.0f) * radius;
		//float cam_z = sin(time_val / 4.0f) * radius;
		glm::mat4 view;
		//view = glm::lookAt(glm::vec3(cam_x, 0.0f, cam_z),
		//				   glm::vec3(0.0f, 0.0f, 0.0f),
		//				   glm::vec3(0.0f, 1.0f, 0.0f));
		view = camera.createView();
		float far_plane = 5000.0f;
		//shader.use();
		//shader.setFloat("far", far_plane);
		//fish_shader.use();
		//fish_shader.setFloat("far", far_plane);
		glm::mat4 proj = glm::perspective(camera.getFOV(), (float) (screen_width) / (float) (screen_height), 0.1f, far_plane);

		// update view and projection uniform buffer
		renderer->updateUniformBuffer(view, proj);

		renderer->draw();

		print(glGetError());

		// skybox rendering
		//glDisable(GL_DEPTH_TEST);
		//{
		//	skybox_shader.use();

		//	glm::mat4 cubemap_view = glm::mat4(glm::mat3(view));

		//	//glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(cubemap_view));
		//	//glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

		//	glBindVertexArray(cubemapVAO);
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}
		//glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// shader.use();
		// uniform

		// shader.setVec3("fogColor", clear_col);

		// spot_light->uniformShader(shader, "spotlight");

		

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texture2);

		// shader.setInt("material.diffuse1", 0);

		// glBindVertexArray(VAO);
		// MVP
		//glm::mat4 view = glm::mat4(1.0f);
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

		//for (int i = 1; i < 9; ++i)
		//{
		//	glm::mat4 model = glm::mat4(1.0f);
		//	model = glm::translate(model, cubePositions[i]);
		//	//model = glm::rotate(model, time_val, glm::vec3(0.5f, 1.0f, 0.0f));

		//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		//	//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));

		//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//	//glDrawArrays(GL_TRIANGLES, 0, 36);
		//}

		// rendering the ball
		//glBindVertexArray(bVAO);

		//glm::mat4 b_model = glm::mat4(1.0f);
		//b_model = glm::translate(b_model, ball_pos);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(b_model));

		//glDrawElements(GL_TRIANGLES, 6 * ball_width * ball_height, GL_UNSIGNED_INT, 0);


		// instanced rendering the fish
		//fish_shader.use();

		//fish_shader.setFloat("angle", time_val);

		//fish_shader.setVec3("fogColor", clear_col);

		//spot_light->uniformShader(&fish_shader, "spotlight");

		//fish_shader.setVec3("viewPos", camera.m_Pos);

		//fish_model.DrawInstanced(fish_shader, num_fish);
		//for (unsigned int i = 0; i < m_model.meshes.size(); ++i)
		//{
		//	glBindVertexArray(m_model.meshes[i].VAO);
		//	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fish_transforms[i]));

		//	m_model.DrawInstanced(fish_shader, num_fish);
		//	glDrawElementsInstanced(GL_TRIANGLES, m_model.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, num_fish);
		//}
		//unsigned int f_width = 3;
		//unsigned int f_height = 3;
		//for (unsigned int y = 0; y < f_height; ++y)
		//{
		//	for (unsigned int x = 0; x < f_width; ++x)
		//	{
		//		srand(y * f_width + x);
		//		glm::vec3 offs = glm::vec3((float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f);
		//		glm::mat4 model = glm::mat4(1.0f);
		//		//model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
		//		//model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		//		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
		//		model = glm::translate(model, glm::vec3(-(float)x * 3 + offs.x, offs.y * 10.0f - 15.0f, -(float)y * 3 + offs.z - 10.0f));
		//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//		shader.setFloat("angle", time_val + (glm::sin(x * y) * 5));

		//		m_model.Draw(shader);
		//	}
		//}

		//rendering the light
		//lightShader.use();
		///*color.x = sin(time_val * 2.0f);
		//color.y = sin(time_val * 0.7f);
		//color.z = sin(time_val * 1.3f);*/

		//glBindVertexArray(lVAO);

		//for (int i = 0; i < 4; ++i)
		//{
		//	lightShader.setVec3("lightColor", pointLightColors[i]);

		//	glm::mat4 model = glm::mat4(1.0f);
		//	model = glm::translate(model, pointLightPositions[i]);

		//	glUniformMatrix4fv(lightShader.uniformLoc("model"), 1, GL_FALSE, glm::value_ptr(model));
		//	//glUniformMatrix4fv(lightShader.uniformLoc("view"), 1, GL_FALSE, glm::value_ptr(view));
		//	//glUniformMatrix4fv(lightShader.uniformLoc("projection"), 1, GL_FALSE, glm::value_ptr(proj));

		//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//}

		//outline
		//glEnable(GL_STENCIL_TEST);
		//glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//{ // drawing first cube with an outline
		//	// first cube
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D, texture);

		//	glBindVertexArray(VAO);

		//	glStencilFunc(GL_ALWAYS, 1, 0xFF);
		//	glStencilMask(0xFF);

		//	shader.use();

		//	glm::mat4 model = glm::mat4(1.0f);
		//	model = glm::translate(model, cubePositions[0]);
		//	//model = glm::rotate(model, time_val, glm::vec3(0.5f, 1.0f, 0.0f));

		//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		//	//glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));

		//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//	// outline
		//	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//	glStencilMask(0x00);
		//	glDisable(GL_DEPTH_TEST);
		//	outline_shader.use();

		//	glm::mat4 outline_model = glm::mat4(1.0f);
		//	outline_model = glm::translate(outline_model, cubePositions[0]);
		//	outline_model = glm::scale(outline_model, glm::vec3(1.1f, 1.1f, 1.1f));

		//	glUniformMatrix4fv(outlineModelLoc, 1, GL_FALSE, glm::value_ptr(outline_model));
		//	//glUniformMatrix4fv(outlineViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		//	//glUniformMatrix4fv(outlineProjectionLoc, 1, GL_FALSE, glm::value_ptr(proj));

		//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//	glStencilMask(0xFF);
		//	glEnable(GL_DEPTH_TEST);
		//	glDisable(GL_STENCIL_TEST);

		//	shader.use();
		//}

		//skybox_shader.use();
		glBindVertexArray(quadVAO);

		//drawing texture quad
		//glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, interFBO);
		glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screen_shader.use();
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inter_frame_texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

		//check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &msFBO);
	glDeleteFramebuffers(1, &interFBO);

	delete(dir_light);
	delete(spot_light);
	delete(renderer);

	glfwTerminate();

	return 0;
}