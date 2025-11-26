#include <iostream>
#include <fstream>
#include <filesystem>

#include "glew.h"
#include "Mesh.h"
#include "Texture.h"

std::vector<Vertex> vertexTemp;
std::vector<unsigned int> indexTemp;


static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) 
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}


static void copyToImportFolder(std::string sourcePath, std::string destPath)
{
    // Open the source file for binary input
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile)
    {
        std::cerr << "Error opening source file." << std::endl;
    }

    // Open the destination file for binary output
    std::ofstream destinationFile(destPath, std::ios::binary);
    if (!destinationFile)
    {
        std::cerr << "Error opening destination file." << std::endl;
    }

    // Copy contents from source to destination
    destinationFile << sourceFile.rdbuf();

    // Close the files
    sourceFile.close();
    destinationFile.close();
}





MeshFBX::MeshFBX(std::string path, bool importMaterial)
    : materialImported(importMaterial)
{
    
    // Load the model with  ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Error loading FBX file: " << importer.GetErrorString() << std::endl;
    }
    else
    {
        // Process the loaded model
        processNode(scene->mRootNode, scene, glm::mat4(1.0f));

        // Set number of meshes and number of materials asscoiated with model
        nMeshes = scene->mNumMeshes;
        nMaterials = scene->mNumMaterials;

        //meshData.resize(nMeshes);

        // For every mesh in scene, create VAO and VBO 
        for (int i = 0; i < nMeshes; i++)
        {
            glGenVertexArrays(1, &meshData[i].VAO);

            // Bind VAO
            glBindVertexArray(meshData[i].VAO);

            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, meshData[i].vertices.size() * sizeof(Vertex), meshData[i].vertices.data(), GL_STATIC_DRAW);

            // Specify the vertex attribute pointers
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
            glEnableVertexAttribArray(2);

            // Generate index buffer
            glGenBuffers(1, &meshData[i].EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData[i].EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData[i].indices.size() * sizeof(unsigned int), meshData[i].indices.data(), GL_STATIC_DRAW);

            // Unbind VAO to prevent accidental changes
            glBindVertexArray(0);
        }

        // Create materials from FBX scene
        if (materialImported)
        {
            std::cout << "Number of materials : " << scene->mNumMaterials << "\n\n";

            loadMaterials(scene);

        }
      
    }

}



void Mesh::drawVAO()
{
    for (int i = 0; i < nMeshes; i++)
    {
        // Bind the VAO
        glBindVertexArray(meshData[i].VAO);

        // Draw the mesh
        glDrawElements(GL_TRIANGLES, meshData[i].indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}

void MeshFBX::processNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
{
    // Apply the current node's transformation
    glm::mat4 currentTransform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);


    // Process all the node's meshes
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        processMesh(mesh, scene, currentTransform);
    }

    // Recursively process each of the node's children
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene, currentTransform);
    }
}


void MeshFBX::processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& parentTransform)
{
    vertexTemp.clear();
    indexTemp.clear();

    // Process each vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) 
    {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        // Check if the mesh has normals
        if (mesh->HasNormals()) 
        {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        else 
        {
            vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        // Check if the mesh has texture coordinates
        if (mesh->HasTextureCoords(0)) 
        {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else 
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertexTemp.push_back(vertex);
    }


    // Process each face (assume triangles for simplicity)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indexTemp.push_back(face.mIndices[j]);
        }
    }


    // Apply the parent transformation to the vertices
    for (Vertex& vertex : vertexTemp)
    {
        glm::vec4 pos = glm::vec4(vertex.position, 1.0f);
        pos = parentTransform * pos;
        vertex.position = glm::vec3(pos);
    }


    // Create new mesh data object
    meshData.push_back(MeshData());

    // Copy the vertex and index data to it
    meshData.back().vertices = std::vector<Vertex>(vertexTemp);
    meshData.back().indices = std::vector<unsigned int>(indexTemp);

    // Set mesh material index
    meshData.back().materialIndex = mesh->mMaterialIndex;

}



void MeshFBX::loadMaterials(const aiScene* scene)
{
    // Create space for material data
    matData.resize(nMaterials);

    // For every material in FBX scene
    for (unsigned int i = 0; i < nMaterials; i++)
    {
        // The material
        const aiMaterial* material = scene->mMaterials[i];

        //std::cout << "Loading material: " << material->GetName().C_Str() << "\n\n";

        //Set material name
        matData[i].name = material->GetName().C_Str();
        
        
       
        // Get diffuse texture paths
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            processTexture(scene, material, aiTextureType_DIFFUSE, "Diffuse", i);
        }

        // Get normal texture paths
        if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            processTexture(scene, material, aiTextureType_NORMALS, "Normals", i);
        }

        // Get specular texture paths
        if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            processTexture(scene, material, aiTextureType_SPECULAR, "Specular", i);
        }

        // Get diffuse color
        aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

        matData[i].diffuseColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f);

        // Get specular color
        aiColor3D specularColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

        matData[i].specularColor = glm::vec4(specularColor.r, specularColor.g, specularColor.b, 1.0f);


        // Get shininess
        float shine = 0.0f;
        material->Get(AI_MATKEY_SHININESS, shine);

        matData[i].shininess = shine;

        // Get reflectivity
        float reflect = 0.0f;
        material->Get(AI_MATKEY_REFLECTIVITY, reflect);

        matData[i].reflectivity = reflect;

    }

}



void MeshFBX::processTexture(const aiScene* scene, const aiMaterial* material, aiTextureType type, const std::string& typeName, unsigned int index)
{
    // Retrieve the number of textures of the given type
    unsigned int textureCount = material->GetTextureCount(type);

    //std::cout << typeName << " Textures (" << textureCount << "):" << std::endl;

    for (unsigned int i = 0; i < textureCount; i++) 
    {
        aiString texturePath;
        
        // Get path to texture
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texturePath);
        
        // Attempt to get embedded texture
        const aiTexture* texture = scene->GetEmbeddedTexture(texturePath.C_Str());

       // std::cout << "Texture " << i << ": " << texturePath.C_Str() << "\n";

        // Texture is embedded
        if (texture)
        {
            if(type == aiTextureType_DIFFUSE)
                loadEmbeddedTexture(texture, type, index);
            else if(type == aiTextureType_NORMALS)
                loadEmbeddedTexture(texture, type, index);
        }
        else // Texture is on disk
        {
            
            // Set texture path to list
            switch (type)
            {
            case aiTextureType_DIFFUSE:
                matData[index].diffuseTexture.texturePath = texturePath.C_Str();
                matData[index].diffuseTexture.isEmbedded = false;
                matData[index].hasDiffuse = true;

                break;
            case aiTextureType_NORMALS:
                matData[index].normalTexture.texturePath = texturePath.C_Str();
                matData[index].normalTexture.isEmbedded = false;
                matData[index].hasNormals = true;

                break;
            case aiTextureType_SPECULAR:
                matData[index].specularTexture.texturePath = texturePath.C_Str();
                matData[index].specularTexture.isEmbedded = false;
                matData[index].hasSpecular = true;

                break;
            }
        }
    }
}



// TODO: FIX EMBEDDED TEXTURES
void MeshFBX::loadEmbeddedTexture(const aiTexture* texture, aiTextureType type, unsigned int index)
{
    std::cout << "Loading embedded texture...\n";

    unsigned int textureSize = 0;

    // Compressed
    if (texture->mHeight == 0)
    {
        textureSize = sizeof(aiTexel) * texture->mWidth;
    }
    else // Not compressed
    {
        textureSize = sizeof(aiTexel) * texture->mWidth * texture->mHeight;
    }

    aiTexel* texels = static_cast<aiTexel*>(texture->pcData);

    // Set texture path to list
    switch (type)
    {
    case aiTextureType_DIFFUSE:
        matData[index].diffuseTexture.isEmbedded = true;
        matData[index].hasDiffuse = true;

        matData[index].diffuseTexture.embeddedData.data = new aiTexel[textureSize];

        memcpy(matData[index].diffuseTexture.embeddedData.data, texels, textureSize / 4);

        std::cout << "Found embedded diffuse\n";
        break;
    case aiTextureType_NORMALS:
        matData[index].normalTexture.isEmbedded = true;
        matData[index].hasNormals = true;

        matData[index].normalTexture.embeddedData.data = new aiTexel[textureSize];

        memcpy(matData[index].normalTexture.embeddedData.data, texels, textureSize);

        std::cout << "Found embedded normals\n";
        break;
    case aiTextureType_SPECULAR:

        matData[index].specularTexture.isEmbedded = true;
        break;
    }
}




// Generate terrain mesh
MeshTerrain::MeshTerrain(float _size, int _resolution, float _height)
    : size(_size), resolution(_resolution), hWidth(0), hHeight(0), height(_height)
{
    generateMesh(size, resolution);
}

MeshTerrain::~MeshTerrain()
{
    meshData[0].indices.clear();
    meshData[0].vertices.clear();
    meshData.clear();
    heightMap.clear();
}

void MeshTerrain::generateMesh(float _size, int _resolution)
{
    size = _size;
    resolution = _resolution;

    int N = resolution;
    float K = size / static_cast<float>(resolution);

    MeshData data;

    // Calculate half-size to center the plane at the origin
    float halfSize = static_cast<float>(N * K) / 2.0f;

    // Generate vertices
    for (int i = 0; i <= N; ++i)
    {
        for (int j = 0; j <= N; ++j)
        {
            float x = j * K - halfSize;
            float z = i * K - halfSize;

            Vertex vert;
            vert.position = { x, z, 0.0f };
            vert.normal = { 0.0f, 0.0f, -1.0f };
            vert.texCoords = { static_cast<float>(j) / N, static_cast<float>(i) / N };

            data.vertices.push_back(vert);
        }
    }

    // Generate indices
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            // Indices for the current quad
            unsigned int topLeft = i * (N + 1) + j;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (i + 1) * (N + 1) + j;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle
            data.indices.push_back(topLeft);
            data.indices.push_back(bottomLeft);
            data.indices.push_back(topRight);

            // Second triangle
            data.indices.push_back(topRight);
            data.indices.push_back(bottomLeft);
            data.indices.push_back(bottomRight);
        }
    }



    // Create VAO for terrain mesh
    glGenVertexArrays(1, &data.VAO);

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Create index VBO 
    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);

    // Specify the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    // Generate index buffer
    glGenBuffers(1, &data.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    // Unbind VAO to prevent accidental changes
    glBindVertexArray(0);

    meshData.push_back(data);

    // Set number of meshes and number of materials asscoiated with model
    nMeshes = 1;
    nMaterials = 0;
}


void MeshTerrain::applyHeightMap()
{
    std::vector<Vertex>& verts = meshData[0].vertices;

    int nVerts = meshData[0].vertices.size();
    int nPixels = heightMap.size();

    const float heightOffset = 2.0f;

    for (int i = 0; i < nVerts; i++)
    {
        glm::vec3& pos = meshData[0].vertices[i].position;
        glm::vec3& norm = meshData[0].vertices[i].normal;

        // Normalized coord
        float nx = (pos.x + size * 0.5f) / size;
        float ny = (pos.y + size * 0.5f) / size;

        // Texture coords
        int u = static_cast<int>(nx * hWidth);
        int v = static_cast<int>(ny * hHeight);

        int index = v * hWidth + u;

        if (index < nPixels && index >= 0)
        {
            unsigned char heightVal = heightMap[index];

            pos.z += static_cast<float>(heightVal) * height;
        }
    }


    // Update normals triangle by triangle
    for (unsigned int i = 0; i < meshData[0].indices.size(); i += 3)
    {
        glm::vec4 minPoint = { 9999.0f, 9999.0f, 9999.0f, 1.0f };
        glm::vec4 maxPoint = { -9999.0f, -9999.0f, -9999.0f, 1.0f };
        glm::vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
        glm::vec3 dim = { 0.0f, 0.0f, 0.0f };

        // Indices of triangle
        unsigned int index0 = meshData[0].indices[i];
        unsigned int index1 = meshData[0].indices[i + 1];
        unsigned int index2 = meshData[0].indices[i + 2];

        // Triangle vertices 
        glm::vec3 A = meshData[0].vertices[index0].position;
        glm::vec3 B = meshData[0].vertices[index1].position;
        glm::vec3 C = meshData[0].vertices[index2].position;

        // Calculate new normal
        glm::vec3 AB = B - A;
        glm::vec3 AC = C - A;

        glm::vec3 newNormal = glm::normalize(glm::cross(AC, AB));

        // Set the new normal
        meshData[0].vertices[index0].normal = newNormal;
        meshData[0].vertices[index1].normal = newNormal;
        meshData[0].vertices[index2].normal = newNormal;
    }

    // Finally, update the mesh on the GPU
    updateMesh();
}


void MeshTerrain::updateTerrain(float newSize, int newRes, float newHeight)
{
    // Update terrain data
    size = newSize;
    resolution = newRes;
    height = newHeight;

    // Delete existing mesh data
    glDeleteVertexArrays(1, &meshData[0].VAO);
    glDeleteBuffers(1, &meshData[0].VBO);
    glDeleteBuffers(1, &meshData[0].EBO);
    meshData.clear();

    // Generate a new mesh
    generateMesh(size, resolution);

    // Apply the height map
    applyHeightMap();
}



void MeshTerrain::updateMesh()
{
    glBindBuffer(GL_ARRAY_BUFFER, meshData[0].VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, meshData[0].vertices.size() * sizeof(Vertex), meshData[0].vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshTerrain::loadHeightMap(std::string path)
{
    heightMap.clear();
    heightMap = Texture::getHeightMapData(path, &hWidth, &hHeight);
}