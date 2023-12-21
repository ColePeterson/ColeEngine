#pragma once

#ifndef _MESH
#define _MESH

#include "transform.h"
#include <vector>
#include <string>
#include <unordered_map>

#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Material;


// A single vertex
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct EmbeddedTextureInfo
{
    aiTexel* data = nullptr;
    unsigned int width, height;
};

struct TextureInfoFBX
{
    std::string texturePath;

    EmbeddedTextureInfo embeddedData;

    bool isEmbedded = false;
};

// Per material data
struct MaterialData
{
    std::string name;

    bool hasDiffuse = false;
    bool hasNormals = false;
    bool hasSpecular = false;

    TextureInfoFBX diffuseTexture;
    TextureInfoFBX specularTexture;
    TextureInfoFBX normalTexture;

    float shininess;
    float reflectivity;

    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
};


// Per mesh data
struct MeshData
{
    unsigned int VAO, EBO;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int materialIndex = 0;
};



// Contains all mesh VAO's and vertices and materials of an FBX scene
class Mesh
{
public:
    Mesh() {};
    virtual ~Mesh() {}

    virtual void drawVAO();

    // Data for every mesh in FBX scene
    std::vector<MeshData> meshData;

    // Data for every material in FBX scene
    std::vector<MaterialData> matData;

    // Number of meshes and materials in scene
    unsigned int nMeshes, nMaterials;
};


class MeshFBX : public Mesh
{
public:
    MeshFBX(std::string path, bool importMaterial);

private:
    void processNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    void processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& parentTransform);
    void processTexture(const aiScene* scene, const aiMaterial* material, aiTextureType type, const std::string& typeName, unsigned int index);

    void loadEmbeddedTexture(const aiTexture* texture, aiTextureType type, unsigned int index);

    void loadMaterials(const aiScene* scene);

    bool materialImported;
};







#endif