#include "World.h"
#include "Shapes.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Logging.h"


#include <algorithm>
#include <glm/ext.hpp>


World::World(Platform& _platform, ResourceManager& _resource)
    : platform(_platform), resource(_resource), last_time(glfwGetTime())
{

}

World::~World()
{

}

void World::initWorld()
{
    front = 0.1f;
    back = 5000.0f;
    ry = 0.4;

    time = 0.0f;

    last_time = glfwGetTime();
    time_dx = 0.0f;

    ambientStrength = 0.2f;

    lightPos = { -8.0f, 7.5f, 20.5f };
    ambientColor = { 0.01f, 0.01f, 0.01f };

    triangleCount = 0;
    bvh = nullptr;

    // Make VAO's for basic shapes
    //Shape* sphereShape = new Sphere(24);
   
    Mesh* sponzaMesh = new MeshFBX("assets/mesh/playground.fbx", false);
    Mesh* monkeyMesh = new MeshFBX("assets/mesh/monkey.fbx", false);
   // Mesh* matTestMesh = new MeshFBX("assets/mesh/diffuseCube.fbx", true);
   // Mesh* skyMesh = new MeshFBX("assets/mesh/skySphere.fbx", false);

    //MeshFBX* temp = reinterpret_cast<MeshFBX*>(matTestMesh);

    
    // Load all shaders
    resource.loadShader("geometry_default", "geo_pass.frag", "geo_pass.vert");
    resource.loadShader("lighting", "lighting.frag", "lighting.vert");
    resource.loadShader("point_shadows_default", "PointLightShadow.frag", "PointLightShadow.vert", "PointLightShadow.geom");
  //  resource.loadShader("shadows_default", "shadow.frag", "shadow.vert");
    //resource.loadShader("skydome", "skydome.frag", "skydome.vert");

    // Create sky material
    //resource.createNewMaterial("mat_sky", resource.shader("skydome"));
    //resource.getMaterial("mat_sky")->vTexture["textureSky"] = resource.getTexture("assets/textures/hdri/kloofendal_43d_clear_2k.hdr"); // Normals texture

    // Set sky texture
    resource.setSkyTexture(resource.getTexture("assets/textures/hdri/kloofendal_43d_clear_2k.hdr"));

    // FBX material import test
    //Material* importMat = temp->loadMaterial();
    //importMat->setShader(resource.shader("geometry_default"));


    /*
    // Create grass material
    resource.createNewMaterial("mat_grass", resource.shader("geometry_default"));

    // Create grass material's parameters
    resource.getMaterial("mat_grass")->vTexture["textureDiffuse"] = resource.getTexture("assets/textures/grass_1.png"); // Diffuse texture
    resource.getMaterial("mat_grass")->vTexture["textureNormal"] = resource.getTexture("assets/textures/grass_1_NRM.png"); // Normals texture
    resource.getMaterial("mat_grass")->vVec2["textureScale"] = Vec2Param(glm::vec2(40.0f, 40.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
    resource.getMaterial("mat_grass")->vFloat["shininess"] = FloatParam(120.0f, 0.2f, 300.0f);
    resource.getMaterial("mat_grass")->vFloat["normalStrength"] = FloatParam(0.1f, 0.0f, 0.5f);
    resource.getMaterial("mat_grass")->vColor["diffuse"] = Color3(1.0f, 1.0f, 1.0f); // Diffuse color
    resource.getMaterial("mat_grass")->vColor["specular"] = Color3(1.0f, 1.0f, 1.0f); // Specular color
    */

    
    // Create concrete material
    resource.createNewMaterial("mat_concrete", resource.shader("geometry_default"));

    // Create concrete material's parameters
    resource.getMaterial("mat_concrete")->vTexture["textureDiffuse"] = resource.getTexture("assets/textures/concrete_1.png");
    resource.getMaterial("mat_concrete")->vTexture["textureNormal"] = resource.getTexture("assets/textures/concrete_1_NRM.png");
    resource.getMaterial("mat_concrete")->vVec2["textureScale"] = Vec2Param(glm::vec2(5.0f, 5.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
    resource.getMaterial("mat_concrete")->vFloat["shininess"] = FloatParam(120.0f, 0.2f, 500.0f);
    resource.getMaterial("mat_concrete")->vFloat["normalStrength"] = FloatParam(0.1f, 0.0f, 0.5f);
    resource.getMaterial("mat_concrete")->vColor["diffuse"] = Color3(1.0f, 1.0f, 1.0f); // Diffuse color
    resource.getMaterial("mat_concrete")->vColor["specular"] = Color3(1.0f, 1.0f, 1.0f); // Specular color



    // Create brick material
    resource.createNewMaterial("mat_brick", resource.shader("geometry_default"));

    // Create brick material's parameters
    resource.getMaterial("mat_brick")->vTexture["textureDiffuse"] = resource.getTexture("assets/textures/brick_1.png");
    resource.getMaterial("mat_brick")->vTexture["textureNormal"] = resource.getTexture("assets/textures/brick_1_NRM.png");
    resource.getMaterial("mat_brick")->vVec2["textureScale"] = Vec2Param(glm::vec2(3.0f, 3.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
    resource.getMaterial("mat_brick")->vFloat["shininess"] = FloatParam(120.0f, 0.2f, 500.0f);
    resource.getMaterial("mat_brick")->vFloat["normalStrength"] = FloatParam(40.0f, 0.0f, 80.0f);
    resource.getMaterial("mat_brick")->vColor["diffuse"] = Color3(1.0f, 1.0f, 1.0f); // Diffuse color
    resource.getMaterial("mat_brick")->vColor["specular"] = Color3(1.0f, 1.0f, 1.0f); // Specular color
    
    

    // Create point light component 2
    PointLightComponent* plComp2 = new PointLightComponent();
    plComp2->color = { 0.9f, 1.0f, 0.9f };
    plComp2->resolution = { 1024, 1024 };
    plComp2->strength = 60.0f;
    plComp2->falloff = 2.0f;
    plComp2->disableShadows = false;
    plComp2->farPlane = 180.0f;

    // Create point light transform component 2
    TransformComponent* plTrn2 = new TransformComponent();
    plTrn2->pos = { 9.0f, -2.0f, 4.0f };
    plTrn2->scl = { 1.0f, 1.0f, 1.0f };

    // Create point light 2
    PointLight* light2 = new PointLight("Point light 2", 1024, 1024);

    light2->addComponent(plComp2);
    light2->addComponent(plTrn2);

    pointLights.push_back(light2);
    

    // Create point light component 2
    PointLightComponent* plComp1 = new PointLightComponent();
    plComp1->color = { 0.1f, 1.0f, 0.1f };
    plComp1->resolution = { 1024, 1024 };
    plComp1->strength = 60.0f;
    plComp1->falloff = 2.0f;
    plComp1->disableShadows = false;
    plComp1->farPlane = 180.0f;

    // Create point light transform component 2
    TransformComponent* plTrn1 = new TransformComponent();
    plTrn1->pos = { -9.0f, -2.0f, 4.0f };
    plTrn1->scl = { 1.0f, 1.0f, 1.0f };

    // Create point light 2
    PointLight* light1 = new PointLight("Point light 1", 1024, 1024);

    light1->addComponent(plComp1);
    light1->addComponent(plTrn1);

    pointLights.push_back(light1);

    // Create plane entity
    Entity* terrainEntity = new Entity("Sponza");

    // Transform for sponza entity
    TransformComponent* trTerrain = new TransformComponent();
    trTerrain->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    //trTerrain->scl = glm::vec3(0.1f, 0.1f, 0.1f);
    trTerrain->scl = glm::vec3(1.0f, 1.0f, 1.0f);
    // Renderer for plane entity
    RenderComponent* rndrSponzaTerrain = new RenderComponent();
    rndrSponzaTerrain->mesh = sponzaMesh;

    rndrSponzaTerrain->setMaterial(resource.getMaterial("mat_brick"), terrainEntity);


    // Add components to plane entity
    terrainEntity->addComponent(trTerrain);
    terrainEntity->addComponent(rndrSponzaTerrain);

    // Add plane to world
    entities.push_back(terrainEntity);
    


    
    
    // Create skybox entity
    Entity* skyBox = new Entity("sky dome");

    // Transform for skybox entity
    TransformComponent* trSky = new TransformComponent();
    trSky->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    trSky->scl = glm::vec3(50.0f, 50.0f, 50.0f);

    // Renderer for skybox entity
    RenderComponent* rndrSky = new RenderComponent();
    //rndrSky->mesh = skyMesh;
    rndrSky->setMaterial(resource.getMaterial("mat_brick"), skyBox);

    // Add components to skybox entity
    skyBox->addComponent(rndrSky);
    skyBox->addComponent(trSky);

    // Add skybox to world
    //entities.push_back(skyBox);
    


    // Monkey entity
    Entity* monk = new Entity("monkey");

    TransformComponent* trMonk = new TransformComponent();
    trMonk->pos = glm::vec3(-5.0f, 6.0f, 2.0f);
    //trMonk->scl = glm::vec3(0.01f, 0.01f, 0.01f);
    trMonk->scl = glm::vec3(1.0f, 1.0f, 1.0f);
    RenderComponent* rndrMonk = new RenderComponent();
    rndrMonk->mesh = monkeyMesh;

    rndrMonk->setMaterial(resource.getMaterial("mat_concrete"), monk);
    //rndrMonk->setMaterial(importMat, monk);

    monk->addComponent(trMonk);
    monk->addComponent(rndrMonk);

    entities.push_back(monk);
    

    
   
    
    

    // Create player entity
    Entity* player = new Entity("player");

    // Transform for player entity
    TransformComponent* trPlayer = new TransformComponent();
    trPlayer->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    trPlayer->scl = glm::vec3(0.18f, 0.18f, 0.18f);
    
    PlayerComponent* playerComp = new PlayerComponent();

    player->addComponent(trPlayer);
    player->addComponent(playerComp);
    
    entities.push_back(player);


   // Log::info("Creating AABB list...");
    createAABBlist();

   // Log::info("Creating BVH tree...");
    //bvh = createBVH(objList, 0);

}

void World::updateTransforms(glm::vec3 eye, float tilt, float spin)
{
    worldView = Rotate(0, tilt - 90) * Rotate(2, spin) * Translate(-eye[0], -eye[1], -eye[2]);
    worldProj = Perspective((ry * platform.width) / platform.height, ry, front, back);
    worldInverse = glm::inverse(worldView);
    eyePos = eye;
}



void World::update()
{
    float now = glfwGetTime();
    time_dx = now - last_time;
    time += time_dx;
    last_time = now;
}


void World::createAABBlist()
{
    objList.clear();

    triangleCount = 0;
    for (Entity* e : entities)
    {
        TransformComponent* transformComp = e->getComponent<TransformComponent>();
        RenderComponent* renderComp = e->getComponent<RenderComponent>();
        PlayerComponent* playerComp = e->getComponent<PlayerComponent>();

        if (!renderComp)
            continue;

        glm::mat4 modelTr = glm::mat4(1.0f);
        modelTr = glm::translate(modelTr, transformComp->pos);
        modelTr = glm::rotate(modelTr, (transformComp->angle), transformComp->rot);
        modelTr = glm::scale(modelTr, transformComp->scl * glm::vec3(0.01f, 0.01f, 0.01f));

        glm::vec3 minPoint = { 9999.0f, 9999.0f, 9999.0f };
        glm::vec3 maxPoint = { -9999.0f, -9999.0f, -9999.0f};
        glm::vec3 centerPoint = { 0.0f, 0.0f, 0.0f};
        glm::vec3 dim = { 0.0f, 0.0f, 0.0f };

        std::vector<std::vector<Vertex>>& vertices = renderComp->mesh->vertices;
        std::vector < std::vector<unsigned int>>& indices = renderComp->mesh->indices;

        for (int i = 0; i < vertices.size(); i++)
        {
            for (int j = 0; j < vertices[i].size(); j++)
            {
                glm::vec3& p = vertices[i][j].position;
                glm::vec4 p4 = modelTr * glm::vec4( p.x, p.y, p.z, 1.0f);

                minPoint = glm::min(glm::vec3(p4.x, p4.y, p4.z), minPoint);
                maxPoint = glm::max(glm::vec3(p4.x, p4.y, p4.z), maxPoint);
            }
        }

        dim.x = glm::abs(maxPoint.x - minPoint.x);
        dim.y = glm::abs(maxPoint.y - minPoint.y);
        dim.z = glm::abs(maxPoint.z - minPoint.z);

        centerPoint = maxPoint - dim * 0.5f;

        Box3D* box = new Box3D(centerPoint, dim * 0.5f, reinterpret_cast<void*>(e));

        AABBComponent* aabb = new AABBComponent();

        if (aabb)
        {
            aabb->box = box;
            e->addComponent(aabb);
        }
    }

}




static Box3D* computeBV(std::vector<Box3D*>& objects)
{
    Box3D* box = new Box3D();

    if (box)
    {
        glm::vec3 minPoint = { 999999.0f, 999999.0f, 999999.0f };
        glm::vec3 maxPoint = { -999999.0f, -999999.0f, -999999.0f };
        glm::vec3 avg = { 0.0f, 0.0f, 0.0f };

        unsigned int size = objects.size();

        for (unsigned int i = 0; i < size; i++)
        {
            glm::vec3 center = objects[i]->center;

            minPoint = glm::min(minPoint, center - objects[i]->extents * 0.25f);
            maxPoint = glm::max(maxPoint, center + objects[i]->extents * 0.25f);

            avg += center;
        }

        avg /= glm::vec3((float)size, (float)size, (float)size);

        box->center = avg;
        box->extents = glm::abs(maxPoint - minPoint) / 2.0f;

        return box;
    }
    else
        return nullptr;

}


// Sorts the list of objects based on the center point of the aabb olong the axis of greatest spread (the greatest dimension of the aabb)
static void sortObjects(Box3D* bv, std::vector<Box3D*>& objects)
{
    if (bv)
    {
        if (bv->extents.x > bv->extents.y && bv->extents.x > bv->extents.z) // Split axis is X axis
        {
            std::sort(objects.begin(), objects.end(), [&](Box3D* a, Box3D* b) {return a->center.x < b->center.x; });
        }
        else if (bv->extents.y > bv->extents.x && bv->extents.y > bv->extents.z) // Split axis is Y axis
        {
            std::sort(objects.begin(), objects.end(), [&](Box3D* a, Box3D* b) {return a->center.y < b->center.y; });
        }
        else if (bv->extents.z > bv->extents.x && bv->extents.z > bv->extents.y) // Split axis is Z axis
        {
            std::sort(objects.begin(), objects.end(), [&](Box3D* a, Box3D* b) {return a->center.z < b->center.z; });
        }
    }
}


TreeNode* World::createBVH(std::vector<Box3D*>& objects, int depth)
{

    // Reached the end, make leaf node and stop recursion
    if (objects.size() <= 1)
    {
        TreeNode* leaf = new TreeNode();
        leaf->box = objects[0];
        leaf->type = NodeType::LEAF;
        num_bvh_leaf_nodes++;

        if (depth < minLeafDepth)
            minLeafDepth = depth;

        if (depth > maxLeafDepth)
            maxLeafDepth = depth;

        return leaf;
    }

    // Index of object that's roughly in the middle 
    int offset = objects.size() / 2;

    // Create a bounding box for this set of objects
    Box3D* levelBV = computeBV(objects);

    // Sort list of objects based on axis of greatest spread
    sortObjects(levelBV, objects);

    // Create two new lists of objects based on the split point
    std::vector<Box3D*> listLeft(objects.begin(), objects.begin() + offset); // First object to halfway point
    std::vector<Box3D*> listRight(objects.begin() + offset, objects.end()); // Halfway point to last object

    // Create the two children nodes by recursion
    TreeNode* leftChild = createBVH(listLeft, ++depth);
    TreeNode* rightChild = createBVH(listRight, ++depth);

    // The node with the bounding box around both its children nodes
    TreeNode* node = new TreeNode();
    node->box = levelBV;
    node->right = rightChild;
    node->left = leftChild;
    node->type = NodeType::INTERNAL;

    return node;

}

void World::testRayAgainstNode(Ray3D ray, TreeNode* node)
{
    if (node)
    {
        Box3D* box = node->box;

        if (node->box)
        {
            num_bvh_intersections++;
            float t = 0.0f;
            if (Intersects(ray, *box, &t))
            {
                if (node->type == NodeType::LEAF)
                {
                    minDepth = glm::min(minDepth, t);
                }
                else if (node->type == NodeType::INTERNAL)
                {
                    testRayAgainstNode(ray, node->left);
                    testRayAgainstNode(ray, node->right);
                }
            }
        }
        else
            return;
    }
}
