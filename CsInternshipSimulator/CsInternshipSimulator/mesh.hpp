

#ifndef MESH_H
#define MESH_H

#include <GL/glew.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"

#include <string>
#include <vector>
using namespace std;

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    glm::vec3 kd = glm::vec3(1.0f);
    glm::vec3 ks = glm::vec3(0.04f);
    float ns = 32.0f;

    unsigned int VAO;

    

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
        glm::vec3 kd, glm::vec3 ks, float ns)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->kd = kd;
        this->ks = ks;
        this->ns = ns;

        setupMesh();
    }

    // render the mesh
    void Draw(Shader& shader)
    {
        bool hasDiffuse = false;
        bool hasSpec = false;

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;

        // bind textures that exist
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            string name = textures[i].type;
            string number;

            if (name == "uDiffMap") { number = std::to_string(diffuseNr++); hasDiffuse = true; }
            else if (name == "uSpecMap") { number = std::to_string(specularNr++); hasSpec = true; }
            else continue;

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // material uniforms (THIS is where Ks/Ns finally get used)
        shader.setBool("uHasDiffuseMap", hasDiffuse);
        shader.setBool("uHasSpecMap", hasSpec);
        shader.setVec3("uKd", kd);
        shader.setVec3("uKs", ks);
        shader.setFloat("uNs", ns);
        // if no diffuse map, the shader will use uKd (so do NOT bind a white texture)
        // but still ensure sampler points to 0 (harmless)
        shader.setInt("uDiffMap1", 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    }
};
#endif

