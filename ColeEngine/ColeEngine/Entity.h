#pragma once


#ifndef _ENTITY
#define _ENTITY


#include<iostream>
#include<vector>
#include<cassert>
#include <unordered_map>

#include "Component.h"

#include <glm/glm.hpp>

class Component;
enum class ComponentType;

class Entity
{
public:
	Entity()
		: id(0), name("Empty entity")
	{
	
	}
	Entity(std::string _name);
	~Entity();

	void addComponent(Component* comp);
	void setName(std::string _name) { name = _name; }

	template <typename T>
	T* getComponent();

	std::string getName() { return name; }
	int getID() { return id; }

	std::unordered_map<ComponentType, Component*> components;

	std::string name;
private:

	int id;
	

};

template<typename T>
inline T* Entity::getComponent()
{
	ComponentType key = T::TYPE;

	std::unordered_map<ComponentType, Component*>::const_iterator got = components.find(key);

	if (got == components.end())
		return nullptr;
	else
		return static_cast<T*>(components[key]);
}



#endif