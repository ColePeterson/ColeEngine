////////////////////////////////////////////////////////////////////////
// A small library of object shapes (ground plane, sphere, and the
// famous Utah teapot), each created as a Vertex Array Object (VAO).
// This is the most efficient way to get geometry into the OpenGL
// graphics pipeline.
//
// Each vertex is specified as four attributes which are made
// available in a vertex shader in the following attribute slots.
//
// position,        vec4,   attribute #0
// normal,          vec3,   attribute #1
// texture coord,   vec3,   attribute #2
// tangent,         vec3,   attribute #3
//
// An instance of any of these shapes is create with a single call:
//    unsigned int obj = CreateSphere(divisions, &quadCount);
// and drawn by:
//    glBindVertexArray(vaoID);
//    glDrawElements(GL_TRIANGLES, vertexcount, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
////////////////////////////////////////////////////////////////////////

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include <glew.h>

#include "Logging.h"



#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "math.h"
#include "Shapes.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tinyobj/tiny_obj_loader.h"

#include "glew.h"

const float PI = 3.14159f;
const float rad = PI / 180.0f;


#define LOG_GL_ERROR() logGlError(__FILE__, __LINE__)


void logGlError(const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* errorString = reinterpret_cast<const char*>(gluErrorString(error));
        std::cerr << "OpenGL Error (" << file << ":" << line << "): " << errorString << std::endl;
    }
}


void pushquad(std::vector<glm::ivec3>& Tri, int i, int j, int k, int l)
{
    Tri.push_back(glm::ivec3(i, j, k));
    Tri.push_back(glm::ivec3(i, k, l));
}

void pushtri(std::vector<glm::ivec3>& Tri, int i, int j, int k)
{
    Tri.push_back(glm::ivec3(i, j, k));
}


// Batch up all the data defining a shape to be drawn (example: the
// teapot) as a Vertex Array object (VAO) and send it to the graphics
// card.  Return an OpenGL identifier for the created VAO.
unsigned int VaoFromTris(std::vector<glm::vec4> Pnt,
    std::vector<glm::vec3> Nrm,
    std::vector<glm::vec2> Tex,
    std::vector<glm::vec3> Tan,
    std::vector<glm::ivec3> Tri)
{
    unsigned int vaoID;
    
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
  
    GLuint Pbuff;
    glGenBuffers(1, &Pbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(),
        &Pnt[0][0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (Nrm.size() > 0) {
        GLuint Nbuff;
        glGenBuffers(1, &Nbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Nbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Nrm.size(),
            &Nrm[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (Tex.size() > 0) {
        GLuint Tbuff;
        glGenBuffers(1, &Tbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Tbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * Tex.size(),
            &Tex[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (Tan.size() > 0) {
        GLuint Dbuff;
        glGenBuffers(1, &Dbuff);
        glBindBuffer(GL_ARRAY_BUFFER, Dbuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Tan.size(),
            &Tan[0][0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint Ibuff;
    glGenBuffers(1, &Ibuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * Tri.size(),
        &Tri[0][0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return vaoID;
}



void Shape::MakeVAO()
{
    vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
    LOG_GL_ERROR();
    count = Tri.size();
}

void Shape::DrawVAO()
{
    glBindVertexArray(vaoID);
    LOG_GL_ERROR();
    if (useArrays)
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
        LOG_GL_ERROR();
        
    }
    else
    {
        glDrawElements(GL_TRIANGLES, 3 * count, GL_UNSIGNED_INT, 0);
        LOG_GL_ERROR();
    }

    glBindVertexArray(0);
    LOG_GL_ERROR();
}




////////////////////////////////////////////////////////////////////////
// Generates a sphere of radius 1.0 centered at the origin.
//   n specifies the number of polygonal subdivisions
Sphere::Sphere(const int n)
{
    diffuseColor = glm::vec3(0.5, 0.5, 1.0);
    specularColor = glm::vec3(1.0, 1.0, 1.0);
    shininess = 120.0;

    float d = 2.0f * PI / float(n * 2);
    for (int i = 0; i <= n * 2; i++) {
        float s = i * 2.0f * PI / float(n * 2);
        for (int j = 0; j <= n; j++) {
            float t = j * PI / float(n);
            float x = cos(s) * sin(t);
            float y = sin(s) * sin(t);
            float z = cos(t);
            Pnt.push_back(glm::vec4(x, y, z, 1.0f));
            Nrm.push_back(glm::vec3(x, y, z));
            Tex.push_back(glm::vec2(s / (2 * PI), t / PI));
            Tan.push_back(glm::vec3(-sin(s), cos(s), 0.0));
            if (i > 0 && j > 0) {
                pushquad(Tri, (i - 1) * (n + 1) + (j - 1),
                    (i - 1) * (n + 1) + (j),
                    (i) * (n + 1) + (j),
                    (i) * (n + 1) + (j - 1));
            }
        }
    }
    MakeVAO();

    Pnt.clear();
    Nrm.clear();
    Tex.clear();
    Tan.clear();
    Tri.clear();
}



// Create basic fullscreen quad 
FullScreenQuad::FullScreenQuad()
{
    GLfloat vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    // Texture coordinates
    GLfloat texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    
    // Generate VAO 
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    // Position VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Texture Coords VBO
    GLuint texVBO;
    glGenBuffers(1, &texVBO);

    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);


}



////////////////////////////////////////////////////////////////////////
// Generates a plane with normals, texture coords, and tangent vectors
// from an n by n grid of small quads.  A single quad might have been
// sufficient, but that works poorly with the reflection map.
Plane::Plane(const float r, const int n)
{
    diffuseColor = glm::vec3(0.3, 0.2, 0.1);
    specularColor = glm::vec3(1.0, 1.0, 1.0);
    shininess = 120.0;

    for (int i = 0; i <= n; i++) {
        float s = i / float(n);
        for (int j = 0; j <= n; j++) {
            float t = j / float(n);
            Pnt.push_back(glm::vec4(s * 2.0 * r - r, t * 2.0 * r - r, 0.0, 1.0));
            Nrm.push_back(glm::vec3(0.0, 0.0, 1.0));
            Tex.push_back(glm::vec2(s, t));
            Tan.push_back(glm::vec3(1.0, 0.0, 0.0));
            if (i > 0 && j > 0) {
                pushquad(Tri, (i - 1) * (n + 1) + (j - 1),
                    (i - 1) * (n + 1) + (j),
                    (i) * (n + 1) + (j),
                    (i) * (n + 1) + (j - 1));
            }
        }
    }

    vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
    count = Tri.size();
}






MeshOBJ::MeshOBJ(std::string path)
{
    useArrays = true;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    int nShapes = 0, nVerts = 0;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        //std::cout << "Shape: " << s << "\nn indices: " << shapes[s].mesh.indices.size() << "\nshapes[s].mesh.num_face_vertices.size(): " << shapes[s].mesh.num_face_vertices.size() << "\n\n";

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            
            glm::vec4 pos;
            glm::vec3 nrm;
            glm::vec2 tex;
            
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) 
            {
                //std::cout << "vert: " << v << "\n";
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                
                pos = glm::vec4(vx, vy, vz, 1.0f);

                Pnt.push_back(pos);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                    nrm = glm::vec3(nx, ny, nz);
                    Nrm.push_back(nrm);
                }

                
                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                    tex = glm::vec2(tx, ty);
                    Tex.push_back(tex);
                }

                
                pushtri(Tri, 
                    3 * size_t(idx.vertex_index) + 0, 
                    3 * size_t(idx.vertex_index) + 1, 
                    3 * size_t(idx.vertex_index) + 2
                );
                
                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
         
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }
   
    vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
    count = Pnt.size();

    Pnt.clear();
    Nrm.clear();
    Tex.clear();
    Tan.clear();
    Tri.clear();

}

ObjLoader::ObjLoader(std::string path)
{
    std::ifstream file(path);
    

    if (file)
    {
        std::stringstream iss;
        iss << file.rdbuf();

        //std::istringstream iss(path);

        std::string word;
        while (iss >> word)
        {
            //std::cout << word << "\n";
            if (word == "v")
            {
                iss >> word;
                float x = ::atof(word.c_str());
                iss >> word;
                float y = ::atof(word.c_str());
                iss >> word;
                float z = ::atof(word.c_str());

                Pnt.push_back(glm::vec4(x, y, z, 1.0f));

            }
            else if (word == "vn")
            {
                iss >> word;
                float xn = ::atof(word.c_str());
                iss >> word;
                float yn = ::atof(word.c_str());
                iss >> word;
                float zn = ::atof(word.c_str());

                Nrm.push_back(glm::vec3(xn, yn, zn));
            }
            else if (word == "vt")
            {
                iss >> word;
                float u = ::atof(word.c_str());
                iss >> word;
                float v = ::atof(word.c_str());
               
                Tex.push_back(glm::vec2(u, v));
            }
            else if (word == "f")
            {
                std::vector<int> indices;

                for (int i = 0; i < 4; i++)
                {
                    iss >> word; // Quad index 1

                    int val = 0;
                    for (int j = 0; j < word.size(); j++)
                    {
                        char c = word[j];

                        if (c != '/' && c != ' ')
                        {
                            // Quad index
                            if (val == 0)
                            {
                                indices.push_back(std::stoi(std::string(1,c)));
                            }
                            else if (val == 1) // Texture coord index
                            {

                            }
                            else if (val == 2) // Normal index
                            {

                            }

                            val++;
                        }
                    }
                }

                if (indices.size() >= 4)
                {
                    pushtri(Tri, indices[0], indices[1], indices[2]);
                    /*
                    pushquad(Tri,
                        size_t(indices[1]),
                        size_t(indices[3]),
                        size_t(indices[2]),
                        size_t(indices[0])
                    );
                    */
                }
                else
                    std::cout << "SHITITITI\n\n";
            }

        }

        vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
        count = Pnt.size();

    }

    /*
    for (int i = 0; i < Pnt.size(); i++)
    {
        std::cout << "\nx: " << Pnt[i].x << "\n";
        std::cout << "y: " << Pnt[i].y << "\n";
        std::cout << "z: " << Pnt[i].z << "\n\n";
    }
    */
}

