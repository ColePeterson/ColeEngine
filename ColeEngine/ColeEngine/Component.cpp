#include "Component.h"
#include "Transform.h"

#include <glew.h>




class Material;



void RenderComponent::setMaterial(Material* _material, Entity* _entity)
{


	// Init material on render component
	if (material == nullptr && _material != nullptr)
	{
		material = _material;
		material->addReference(_entity);

		return;
	}

	// Setting a new material that's different from the previous
	if (material != _material)
	{
		// Remove entity from previous material's referernces
		material->removeReference(_entity);

		// Set material to new material
		material = _material;

		// Add entity to new material references
		material->addReference(_entity);

	}
	
}