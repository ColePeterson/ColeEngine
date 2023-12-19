#include "Entity.h"

static int eid = 0;

Entity::Entity(std::string _name)
	: name(_name)
{
	id = eid;
	eid++;
}


Entity::~Entity()
{
	for (const std::pair<ComponentType, Component*>& i : components)
	{
		if(i.second)
			delete i.second;
	}
}


void Entity::addComponent(Component* comp)
{
	if (comp)
	{
		components.insert({ comp->type, comp });
	}

}






