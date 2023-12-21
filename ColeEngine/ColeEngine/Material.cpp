#include "Material.h"
#include "Logging.h"

#include <iostream>
#include <sstream>

Material::Material(std::string _name, ShaderProgram* _shader)
	: name(_name), shader(_shader), vFloat(0), vInt(0), vVec3(0), vTexture(0), vColor(0),
	hasDiffuseTexture(false), hasNormalsTexture(false), hasSpecularTexture(false)
{
	
	
}



void Material::setShader(ShaderProgram* _shader)
{
	if (_shader)
	{
		shader = _shader;
	}
	else
	{
		Log::warning("Attempted to assign NULL shader to material: " + this->getName());
		//std::cout << "Cannot assign null shader to material\n";
	}
}



void Material::addReference(Entity* _entity)
{
	if (_entity)
	{
		entityRefs.push_back(_entity);
	}
}


void Material::removeReference(Entity* _entity)
{
	entityRefs.erase(std::remove(entityRefs.begin(), entityRefs.end(), _entity), entityRefs.end());
}

void Material::registerUniforms()
{
	if (shader)
	{
		bool worldMode = false, noRegister = false;

		std::istringstream iss(shader->shaderSrc);

		std::string word;
		while (iss >> word)
		{
			if (word == "uniform")
			{
				if (!worldMode && !noRegister)
				{
					iss >> word;
					std::string type = word;

					iss >> word;
					std::string name = word;
					name.pop_back();

					//std::cout << "\nUniform of type: " << type << " with name: " << name << "\n";

					if (type == "sampler2D")
					{
						//vTexture[name] = nullptr;
					}
					else if (type == "vec3")
					{
						vVec3[name] = Vec3Param(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
					}
					else if (type == "vec2")
					{
						vVec2[name] = Vec2Param(glm::vec2(1., 1.), glm::vec2(0., 0.), glm::vec2(0., 0.));
					}
					else if (type == "float")
					{
						vFloat[name] = FloatParam(1.0f, 0.0f, 1.0f);
					}
					else if (type == "Color3")
					{
						vColor[name] = glm::vec3(1., 1., 1.);
					}
				}
			}
			else if (word == "//WORLD")
			{
				worldMode = true;
			}
			else if (word == "//WORLD_END")
			{
				worldMode = false;
			}
			else if (word == "//NO_REGISTER")
			{
				noRegister = true;
			}
			else if (word == "//END_NO_REGISTER")
			{
				noRegister = false;
			}

			
		}

	}

}

