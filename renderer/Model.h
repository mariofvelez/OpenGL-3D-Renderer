#pragma once
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <gl_util/Shader.h>

#include "Util.h"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#define print(x) std::cout << x << std::endl

#define get_millis() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

#define calculate_time(x) float t1 = get_millis(); x; float t2 = get_millis(); std::cout << "time: " << (t2 - t1) << "ms" << std::endl


enum TextureType {
	DIFFUSE,
	SPECULAR,
	NORMAL,
	EMISSION
};
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};
struct Texture {
	unsigned int id;
	TextureType type;
	std::string path;
};
struct Mesh {
public:
	unsigned int VAO, VBO, EBO;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::vector<unsigned int> texture_locations;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
		: vertices(vertices), indices(indices), textures(textures)
	{
		setupMesh();
	}
	void Draw(Shader& shader)
	{
		// only sets up shader texture locations on the first draw call
		if (!has_rendered)
		{
			setupShader(shader);
			has_rendered = true;

			print("textures: " << textures.size());
			print("locations: " << texture_locations.size());
		}

		for (unsigned int i = 0; i < texture_locations.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUniform1i(texture_locations[i], i);

			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	void DrawInstanced(Shader& shader, unsigned int instances)
	{
		// only sets up shader texture locations on the first draw call
		if (!has_rendered)
		{
			setupShader(shader);
			has_rendered = true;

			print("textures: " << textures.size());
			print("locations: " << texture_locations.size());
		}

		for (unsigned int i = 0; i < texture_locations.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUniform1i(texture_locations[i], i);

			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instances);
		glBindVertexArray(0);
	}

private:
	bool has_rendered = false;

	void setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		// texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);
	}
	void setupShader(Shader& shader)
	{
		// finds and stores all uniform locations of the texture samplers

		unsigned int diffuse_num = 1;
		unsigned int specular_num = 1;
		unsigned int normal_num = 1;
		unsigned int emission_num = 1;

		for (unsigned int i = 0; i < textures.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			std::string number;
			std::string name;

			switch (textures[i].type)
			{
			case DIFFUSE:
				name = "diffuse";
				number = std::to_string(diffuse_num++);
				break;
			case SPECULAR:
				name = "specular";
				number = std::to_string(specular_num++);
				break;
			case NORMAL:
				name = "normal";
				number = std::to_string(normal_num++);
				break;
			case EMISSION:
				name = "emission";
				number = std::to_string(emission_num++);
			}
			std::string uniform_name = "material." + name + number;
			unsigned int loc = shader.uniformLoc(uniform_name);
			texture_locations.push_back(loc);
			print("found " << uniform_name << " texture at " << loc);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
	}
};
class Model {
public:
	std::vector<Mesh> meshes;

	Model(const std::string& path)
	{
		calculate_time(loadModel(path));
	}
	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); ++i)
		{
			meshes[i].Draw(shader);
		}
	}
	void DrawInstanced(Shader& shader, unsigned int instances)
	{
		for (unsigned int i = 0; i < meshes.size(); ++i)
		{
			meshes[i].DrawInstanced(shader, instances);
		}
	}
private:
	std::vector<Texture> textures_loaded;
	std::string directory;

	void loadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		std::cout << "processing nodes" << std::endl;

		std::cout << "Loaded model" << std::endl;

		processNode(scene->mRootNode, scene);

		print("nodes: " << meshes.size());
	}
	void processNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			print("processing mesh");
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			print("processing node");
			processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		std::vector<unsigned int> indices;
		indices.reserve(mesh->mNumFaces);
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;

			// process vertex data
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;

			if (mesh->mTextureCoords[0]) // if texture coords exist
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = vec;
			}
			else
				vertex.texCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
				indices.push_back(face.mIndices[j]);
		}

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::vector<Texture> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE, DIFFUSE);
			textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

			std::vector<Texture> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR, SPECULAR);
			textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

			std::vector<Texture> normal_maps = loadMaterialTextures(material, aiTextureType_NORMALS, NORMAL);
			textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

			std::vector<Texture> emission_maps = loadMaterialTextures(material, aiTextureType_EMISSIVE, EMISSION);
			textures.insert(textures.end(), emission_maps.begin(), emission_maps.end());
		}

		return Mesh(vertices, indices, textures);
	}
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); ++j)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory, true);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};