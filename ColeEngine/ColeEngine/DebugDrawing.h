#pragma once

#ifndef _DEBUGDRAWING
#define _DEBUGDRAWING


#include<vector>
#include<iostream>

#include "Engine.h"
#include "Entity.h"
#include "Component.h"
#include "Shader.h"

#include <glm/ext.hpp>

// contains methods for drawing AABB bounding box around some object
class DebugAABB
{
public:

    // Constructor for AABB bounding box. Finds min/max and center points of referenced object
    DebugAABB()
        : color(glm::vec3(0.9f, 0.9f, 0.9f)), needsUpdate(true)
    {}
   
    // Destructor
    ~DebugAABB() {}

    void createVAO();
    void draw(Entity* entity, ShaderProgram* program);

    glm::vec3 color; 

    glm::mat4 transform;

    unsigned int vaoID;

    bool needsUpdate;

private:
    glm::mat4 makeTransformFromEntity(Entity* entity);
};


// contains methods for drawing bounding sphere around some object
class DebugSphere
{
public:

    // Constructor for bounding sphere.
    DebugSphere()
        : color({ 0.0f, 1.0f, 0.0f }), center({ 0.0f, 0.0f, 0.0f }), needsUpdate(true)
    {

    };

    // Destructor
    ~DebugSphere() {};

    void createVAO(unsigned int N);
    void draw(Entity* entity, glm::vec3 eye, ShaderProgram* program);
   

    unsigned int count; // Number of vertices

    float radius; // Sphere radius

    glm::vec3 color; // Debug line color
    glm::vec3 center; // Center point

    // Circle transforms
    glm::mat4 trZ;
    glm::mat4 trY;
    glm::mat4 trX;
    glm::mat4 trS;

    // VAO ID for this object
    unsigned int vaoID;

    bool needsUpdate;

private:
    void makeTransformFromEntity(Entity* entity);
    void updateSilhouette(glm::vec3 E);
};






// contains methods for drawing wireframe triangles for a mesh
class DebugTriangles
{
public:

    DebugTriangles()
        : color({ 0.0f, 1.0f, 1.0f })
    {

    };
    ~DebugTriangles() {};

    void createVAO();
    void draw(Entity* entity, ShaderProgram* program);
    
    std::vector<glm::vec4> Pnt; // Vertex list
    std::vector<int> indices; // Index list

    glm::vec3 color; // Debug line color

    // VAO ID for this object
    unsigned int vaoID;
};







class DebugGimbal
{
public:
    DebugGimbal()
        : color({ 1.0f, 1.0f, 1.0f })
    {

    };

    void createVAO();
    void draw(Entity* entity, ShaderProgram* program);
    void draw(glm::vec3 pos, ShaderProgram* program);

    glm::mat4 makeTransform(Entity* entity);

    std::vector<glm::vec4> Pnt; // Vertex list
    std::vector<int> indices; // Index list

    glm::mat4 transform;

    // VAO ID for this object
    unsigned int vaoID;

    glm::vec3 color; // Debug line color

};









#endif