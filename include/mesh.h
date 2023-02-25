#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>
using namespace std;
using namespace glm;

#define NUM_BONES_PER_VERTEX 4
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define INVALID_MATERIAL 0xFFFFFFFF

struct Vertex
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
};

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

struct MeshEntry
{
	MeshEntry()
	{
		NumIndices = 0;
		BaseVertex = 0;
		BaseIndex = 0;
		MaterialIndex = INVALID_MATERIAL;
	}

	unsigned int NumIndices;
	unsigned int BaseVertex;
	unsigned int BaseIndex;
	unsigned int MaterialIndex;
};

vector<MeshEntry> m_Entries;
vector<GLuint> m_Textures;

struct BoneInfo
{
	mat4 offset;
	mat4 FinalTransformation;
	fdualquat FinalTransformationDQ;
	BoneInfo()
	{
		offset = mat4(0.0f);
		FinalTransformation = mat4(0.0f);
		FinalTransformationDQ = fdualquat(quat(1.f, 0.f, 0.f, 0.f), quat(0.f, 0.f, 0.f, 0.f));
	}
};

struct VertexBoneData
{
	unsigned int BoneIDs[NUM_BONES_PER_VERTEX];
	float Weights[NUM_BONES_PER_VERTEX];

	VertexBoneData()
	{
		Reset();
	};

	void Reset()
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
		{
			BoneIDs[i] = 0;
			Weights[i] = 0;
		}
	}
	
	void AddBoneData(unsigned int BoneID, float Weight)
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
			if (Weights[i] == 0.0) {
				BoneIDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}
	}
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
	vector<BoneInfo> bones;
	vector<VertexBoneData> vertexBoneData;    
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, vector<BoneInfo> bones, vector<VertexBoneData> vertexBoneData)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
		this->bones = bones;
		this->vertexBoneData = vertexBoneData;        
		
		setupMesh();
    }

    void Draw(Shader& shader)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);

            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:
    unsigned int vertexData_vbo, EBO, vertexBones_vbo;

    void setupMesh()
    {
		glGenBuffers(1, &vertexData_vbo);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &EBO);
		glGenBuffers(1, &vertexBones_vbo);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, vertexData_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindBuffer(GL_ARRAY_BUFFER, vertexBones_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBoneData) * vertexBoneData.size(), &vertexBoneData[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

        glBindVertexArray(0);
    }
};
#endif