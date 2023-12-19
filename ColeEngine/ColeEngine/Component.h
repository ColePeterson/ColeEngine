#pragma once

#ifndef _COMPONENT
#define _COMPONENT




#include <glm/glm.hpp>

#include<iostream>

#include "Shapes.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "FBO.h"
#include"geomlib.h"
class Mesh;

enum class ComponentType
{
	TransformComponent,
	RenderComponent,
	PlayerComponent,
	PointLightComponent,
	AABBComponent
};

enum class RenderID
{
	NONE,
	GEO,
	SKY
};

class Material;
class Entity;
class ShaderProgram;


#define COMPONENT_COMMON_IMPL( classname )                        \
  constexpr static ComponentType TYPE = ComponentType::classname; \
  classname()													\
  {															\
	type = TYPE;											\
  }													\
  classname( const classname & )      = default;                  \
  classname &   operator=( const classname & ) = default;         \
  ComponentType getType() const                          \
  {                                                               \
    return ComponentType::classname;                              \
  }                                                               \
  







class Component
{
public:
	Component(){};

	virtual ~Component() {};
	virtual void update() {};
	
	ComponentType type;

};



class TransformComponent : public Component
{
public:
	COMPONENT_COMMON_IMPL(TransformComponent);

	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scl = { 1.0f, 1.0f, 1.0f };
	glm::vec3 rot = { 0.0f, 0.0f, 1.0f };

	glm::mat4 transform;

	int axis = 0;
	float angle = 0.0f;

	bool needsUpdate = true;
};

	


class RenderComponent : public Component
{
public:
	COMPONENT_COMMON_IMPL(RenderComponent);

	// object mesh
	Shape* mesh2 = nullptr;
	Mesh* mesh = nullptr;

	// Object material
	Material* material = nullptr;

	void setMaterial(Material* _material, Entity* _entity);
};



class PlayerComponent : public Component
{
public:
	COMPONENT_COMMON_IMPL(PlayerComponent);

	float tilt = 0.0f;
	float spin = 0.0f;

	float walkSpeed = 5.0f;
	float runSpeed = 70.0f;

	glm::vec3 eye = { 0.0f, 0.0f, 4.0f };

};

class PointLightComponent : public Component
{
public:
	COMPONENT_COMMON_IMPL(PointLightComponent);

	// Light color
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };

	// Shadowmap resolution
	glm::ivec2 resolution = { 1024, 1024 };

	// Light strength
	float strength = 10.0f;

	// Light falloff
	float falloff = 1.0f;

	// Light far plane
	float farPlane = 80.0f;

	// Toggle shadows on light
	bool disableShadows = false;

};

class AABBComponent : public Component
{
public:
	COMPONENT_COMMON_IMPL(AABBComponent);

	glm::vec3 color = { 1.0f, 0.0f, 0.0f };
	glm::vec3 dim = { 0.0f, 0.0f, 0.0f };
	glm::vec3 center = { 0.0f, 0.0f, 0.0f };

	bool visible = true;

	Box3D* box = nullptr;

};



/*
RenderComponent& operator=(Entity* _entity)
{
	material->addReference(_entity);

	return *this;
}
*/



#endif