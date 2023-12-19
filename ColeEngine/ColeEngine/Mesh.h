#pragma once

#ifndef _MESH
#define _MESH

#include "transform.h"
#include <vector>
#include <string>

#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Material;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct MaterialData
{
    std::string name;

    std::vector<std::string> diffuseTexturePath;
    std::vector<std::string> specularTexturePath;
    std::vector<std::string> normalTexturePath;

    float shininess;
    float reflectivity;

    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
};


class Mesh
{
public:
    Mesh() {};
    virtual ~Mesh() {}

    std::vector<unsigned int> VAO;
    std::vector<std::vector<Vertex>> vertices;
    std::vector < std::vector<unsigned int>> indices;

    unsigned int nMeshes;

    virtual void drawVAO();
};


class MeshFBX : public Mesh
{
public:
    MeshFBX(std::string path, bool importMaterial);

    Material* loadMaterial();

private:
    void processNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    void processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& parentTransform);
    void processTexture(const aiMaterial* material, aiTextureType type, const std::string& typeName);

    MaterialData* materialData;

    bool materialImported;

    
};

#endif