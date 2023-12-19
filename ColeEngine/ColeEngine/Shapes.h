
#ifndef _SHAPES
#define _SHAPES

#include "transform.h"
#include <vector>
#include <string>



class Shape
{
public:

    // The OpenGL identifier of this VAO
    unsigned int vaoID;

    // Data arrays
    std::vector<glm::vec4> Pnt;
    std::vector<glm::vec3> Nrm;
    std::vector<glm::vec2> Tex;
    std::vector<glm::vec3> Tan;
    std::vector<int> indices;

    // Lighting information
    glm::vec3 diffuseColor, specularColor;
    float shininess;

    // Geometry defined by indices into data arrays
    std::vector<glm::ivec3> Tri;
    unsigned int count;

    // Defined by SetTransform by scanning data arrays
    glm::vec3 minP, maxP;
    glm::vec3 center;
    float size;
    bool animate;

    bool useArrays;

    // Constructor and destructor
    Shape() :animate(false), useArrays(false) {}
    virtual ~Shape() {}

    virtual void MakeVAO();
    virtual void DrawVAO();
};






class MeshOBJ : public Shape
{
public:
    MeshOBJ(std::string path);
};


class ObjLoader : public Shape
{
public:
    ObjLoader(std::string path);
};



class Sphere : public Shape
{
public:
    Sphere(const int n);
};

class Plane : public Shape
{
public:
    Plane(const float range, const int n);
};


class FullScreenQuad
{
public:
    FullScreenQuad();

    unsigned int vaoID;
};





#endif
