#include <iostream>
#include <fstream>

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


MeshFBX::MeshFBX(std::string path, bool importMaterial)
    : materialImported(importMaterial), materialData(nullptr)
{
    
    // Allocate structure for material data from FBX
    if (materialImported)
    {
        materialData = new MaterialData();
    }


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
        nMeshes = scene->mNumMeshes;
        
        // Create a VAO for every mesh in model
        VAO.resize(nMeshes);
        glGenVertexArrays(vertices.size(), VAO.data());

        // Create buffers for every mesh in the model
        for (int i = 0; i < nMeshes; i++)
        {
            // Bind VAO
            glBindVertexArray(VAO[i]);

            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(Vertex), vertices[i].data(), GL_STATIC_DRAW);

            // Specify the vertex attribute pointers
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
            glEnableVertexAttribArray(2);

            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices[i].size() * sizeof(unsigned int), indices[i].data(), GL_STATIC_DRAW);

            // Unbind VAO to prevent accidental changes
            glBindVertexArray(0);
        }

       

        if (materialImported)
        {

        }

    }

}



void Mesh::drawVAO()
{
    for (int i = 0; i < nMeshes; i++)
    {
        // Bind the VAO
        glBindVertexArray(VAO[i]);

        // Draw the mesh
        glDrawElements(GL_TRIANGLES, indices[i].size(), GL_UNSIGNED_INT, 0);

        //glDrawArrays(GL_TRIANGLES, 0, vertices.size());

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


    vertices.push_back(vertexTemp);
    indices.push_back(indexTemp);


    // Store FBX material data to make a material later
    if (materialImported)
    {
        // Access the material for the current mesh
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        materialData->name = material->GetName().C_Str();

        std::cout << "Loaded mesh: " << mesh->mName.C_Str() << "\n";
        std::cout << "Material index: " << mesh->mMaterialIndex << "\n";

        // Get diffuse texture paths
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            processTexture(material, aiTextureType_DIFFUSE, "Diffuse");
        }

        // Get normal texture paths
        if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            processTexture(material, aiTextureType_NORMALS, "Normals");
        }

        // Get specular texture paths
        if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            processTexture(material, aiTextureType_SPECULAR, "Specular");
        }

        // Get diffuse color
        aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        
        materialData->diffuseColor = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);

        // Get specular color
        aiColor3D specularColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

        materialData->specularColor = glm::vec3(specularColor.r, specularColor.g, specularColor.b);


        // Get shininess
        float shine = 0.0f;
        material->Get(AI_MATKEY_SHININESS, shine);

        materialData->shininess = shine;

        // Get reflectivity
        float reflect = 0.0f;
        material->Get(AI_MATKEY_REFLECTIVITY, reflect);

        materialData->reflectivity = reflect;
    }

    
}


void MeshFBX::processTexture(const aiMaterial* material, aiTextureType type, const std::string& typeName)
{
    // Retrieve the number of textures of the given type
    unsigned int textureCount = material->GetTextureCount(type);

    std::cout << typeName << " Textures (" << textureCount << "):" << std::endl;

   

    for (unsigned int i = 0; i < textureCount; i++) 
    {
        aiString texturePath;
        if (material->GetTexture(type, i, &texturePath) == AI_SUCCESS) 
        {
            std::cout << "  Texture Path: " << texturePath.C_Str() << std::endl;

            // Add texture path to list
            switch (type)
            {
            case aiTextureType_DIFFUSE:
                materialData->diffuseTexturePath.push_back(texturePath.C_Str());
                break;
            case aiTextureType_NORMALS:
                materialData->normalTexturePath.push_back(texturePath.C_Str());
                break;
            case aiTextureType_SPECULAR:
                materialData->specularTexturePath.push_back(texturePath.C_Str());
                break;
            }
            
        }
        else 
        {
            std::cerr << "Error retrieving texture path for " << typeName << " texture " << i << std::endl;
        }
    }
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

Material* MeshFBX::loadMaterial()
{
    if (materialData)
    {
        // Create new material
        Material* mat = new Material(materialData->name, nullptr);

        // Set diffuse color
        mat->vColor["diffuse"] = materialData->diffuseColor;

        // Set specular color
        mat->vColor["specular"] = materialData->specularColor;

        // Set shininess
        mat->vFloat["shininess"] = FloatParam(materialData->shininess, 0.0f, 1.0f);

        // Set reflectivity
        mat->vFloat["reflectivity"] = FloatParam(materialData->reflectivity, 0.0f, 1.0f);


        // Load diffuse textures
        for (unsigned int i = 0; i < materialData->diffuseTexturePath.size(); i++)
        {
            Texture* texture = new Texture();
            if (texture)
            {
                // Load image
                texture->load(materialData->diffuseTexturePath[i]);

                // Attach texture to material
                mat->vTexture["Diffuse " + std::to_string(i)] = texture;

                // Copy image to project import folder
                //copyToImportFolder(materialData->diffuseTexturePath[i],
                    //"assets/mesh/imported fbx/" + materialData->name + " diffuse " + std::to_string(i) + ".png");
            }
        }

        // Load normals textures
        for (unsigned int i = 0; i < materialData->normalTexturePath.size(); i++)
        {
            Texture* texture = new Texture();
            if (texture)
            {
                // Load image
                texture->load(materialData->normalTexturePath[i]);

                // Attach texture to material
                mat->vTexture["Normal " + std::to_string(i)] = texture;

                // Copy image to project import folder
                //copyToImportFolder(materialData->diffuseTexturePath[i],
                    //"assets/mesh/imported fbx/" + materialData->name + " normal " + std::to_string(i) + ".png");
            }
        }

        // Load specular textures
        for (unsigned int i = 0; i < materialData->specularTexturePath.size(); i++)
        {
            Texture* texture = new Texture();
            if (texture)
            {
                // Load image
                texture->load(materialData->specularTexturePath[i]);

                // Attach texture to material
                mat->vTexture["Specular " + std::to_string(i)] = texture;

                // Copy image to project import folder
               // copyToImportFolder(materialData->diffuseTexturePath[i],
                   // "assets/mesh/imported fbx/" + materialData->name + " specular " + std::to_string(i) + ".png");
            }
        }

        return mat;
    }
    else
    {
        std::cerr << "Could not import material fom FBX!\n";
        return nullptr;
    }

    
}