#include "Component.h"
#include "Transform.h"

#include <glew.h>




class Material;



void RenderComponent::setMaterial(Material* _material, Entity* _entity, unsigned int index)
{

	if (_material)
	{
		//materials.clear();
		//materials.push_back(_material);

		if (materials.size() == 0)
		{
			materials.push_back(_material);
		}
		else
		{
			if (index < materials.size())
			{
				materials[index] = _material;
			}
		}
	}

}

void RenderComponent::setMaterialsFromMesh(ResourceManager* resource)
{
	if (mesh && resource)
	{
		unsigned int nMaterials = mesh->nMaterials;

		// Fill material slots with references to the materials in model
		for (unsigned int i = 0; i < nMaterials; i++)
		{
			materials.push_back(resource->getMaterial(mesh->matData[i].name));
		}
	}
}