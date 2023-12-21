#include<iostream>

#include "ResourceManager.h"
#include "Material.h"
#include "Logging.h"


ResourceManager::ResourceManager()
    :textures(0), shaders(0), nullTexture(nullptr), skyTexture(nullptr)
{
    nullTexture = new Texture(16, 16, 4, GL_LINEAR);
}

Texture* ResourceManager::getNullTexture()
{
    if (nullTexture)
    {
        return nullTexture;
    }
    else
    {
        Log::error("Cant get null texture");
        return nullptr;
    }
}

void ResourceManager::loadShader(std::string name, std::string fragSrc, std::string vertSrc)
{
    ShaderProgram* shader = new ShaderProgram(name);

    if (shader)
    {
        shader->AddShader(vertSrc, GL_VERTEX_SHADER);
        shader->AddShader(fragSrc, GL_FRAGMENT_SHADER);

        shader->LinkProgram();

        shaders[name] = shader; // Add new shader to list of shaders in resource manager

    }
}

// Create a new shader and add to resource manager
void ResourceManager::loadShader(std::string name, std::string fragSrc, std::string vertSrc, std::string geomSrc)
{
    ShaderProgram* shader = new ShaderProgram(name);

    if (shader)
    {
        shader->AddShader(vertSrc, GL_VERTEX_SHADER);
        shader->AddShader(geomSrc, GL_GEOMETRY_SHADER);
        shader->AddShader(fragSrc, GL_FRAGMENT_SHADER);

        shader->LinkProgram();

        shaders[name] = shader; // Add new shader to list of shaders in resource manager

    }

}
/*
void ResourceManager::loadShader(std::string name, std::string fragSrc, std::string vertSrc, std::string geomSrc)
{

}
*/


// If shader exists, return it.
ShaderProgram* ResourceManager::shader(std::string name)
{
    if (shaders.find(name) != shaders.end()) {
        return shaders[name];
    }
    else {
        std::cout << "Unable to find shader: " << name << " in resource manager\n";
        return nullptr;
    }

}




void ResourceManager::freeShader(std::string name)
{

}

void ResourceManager::addTexture(std::string name, Texture* texture)
{
    if (texture)
    {
        // Texture with this name already exists
        if (textures.count(name) > 0) 
        {
            Log::warning("Cannot add texture to resource manager. Texture with that name already exists");
        }
        else
        {
            // Add the texture to resource manager
            textures[name] = texture;
        }
    }
}

// Create a new texture and add to resource manager
Texture* ResourceManager::loadTexture(std::string src)
{
    // Create new texure
    Texture* texture = new Texture();

    // Allocation success
    if (texture)
    {
        // Texture load success
        if (texture->load(src))
        {
            // Add new texture to resource manager
            textures[src] = texture;

            Log::msg("loaded texture: " + src);

            return textures[src];
        }
        else
        {
            Log::warning("Failed to load texture in resource manager: " + src);
            return nullptr;
        }
    }
    else
    {
        Log::warning("Failed to allocate texture in resource manager: " + src);
        return nullptr;
    }
    
}

// If texture exists, return it. If not, load the texture, add it to resource manager and return it.
Texture* ResourceManager::getTexture(std::string src)
{
    if (textures.find(src) != textures.end()) 
    {
        return textures[src];
    }
    else 
    {
        return loadTexture(src);
    }
}


void ResourceManager::freeTexture(std::string name)
{

}


void ResourceManager::createNewMaterial(std::string name, ShaderProgram* shader)
{
    Material* mat = new Material(name, shader);

    if (mat)
    {
        if (shader)
        {
            mat->setShader(shader);

            mat->registerUniforms();

            Log::msg("Created material " + name);

            materials[name] = mat; // Add new material to list of materials
        }
    }
    else
    {
        Log::warning("Material allocation failed");
    }
}




Material* ResourceManager::getMaterial(std::string name)
{
    if (materials.find(name) != materials.end())
    {
        return materials[name];
    }
    else
    {
        Log::warning("Cannot find material: " + name + " in resource manager");
        return nullptr;
    }
}

