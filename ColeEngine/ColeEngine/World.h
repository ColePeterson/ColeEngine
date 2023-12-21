#pragma once

#ifndef _WORLD
#define _WORLD

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include "Shader.h"
#include "Platform.h"
#include "Transform.h"
#include "Entity.h"
#include "PointLight.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include"geomlib.h"


enum class NodeType
{
	INTERNAL,
	LEAF,
};


struct TreeNode
{
public:

	NodeType type; // Either leaf or internal

	Box3D* box = nullptr; // AABB for this nodes extents

	TreeNode* left = nullptr; // Left child node
	TreeNode* right = nullptr; // Right child node
};




class World
{
public:
	World(Platform& _platform, ResourceManager& _resource);
	~World();

	void initWorld();
	void updateTransforms(glm::vec3 eye, float tilt, float spin);
	void update();

	std::vector<Entity*> entities; // List of all entities in world
	std::vector<PointLight*> pointLights; // Reference list of all point lights in world
	

	float front, back, ry, time, last_time, time_dx;
	
	float ambientStrength;

	glm::vec3 lightPos;
	glm::vec3 ambientColor;
	
	glm::vec3 eyePos;

	glm::mat4 worldView;
	glm::mat4 worldProj;
	glm::mat4 worldInverse;

	TreeNode* bvh;

	float minDepth = 999.0f;

	void testRayAgainstNode(Ray3D ray, TreeNode* node);

private:
	Platform& platform;
	ResourceManager& resource;

	// BVH objects
	std::vector<Box3D*> objList;
	

	int triangleCount;
	int num_bvh_leaf_nodes = 0;
	int minLeafDepth = 999;
	int maxLeafDepth = 0;
	int num_bvh_intersections = 0;

	Mesh* importFBX(std::string path);

	void createAABBlist();
	void createAABBlist2();

	void createBvhObjects();

	TreeNode* createBVH(std::vector<Box3D*>& objects, int depth);

	

};

#endif