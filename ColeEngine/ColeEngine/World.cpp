#include "World.h"
#include "Shapes.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include "Texture.h"
#include "Material.h"
#include "Logging.h"
#include "Serialization.h"

#include "ParticleEmitter.h"

#include <algorithm>
#include <glm/ext.hpp>


World::World(Platform& _platform, ResourceManager& _resource)
    : platform(_platform), resource(_resource), last_time(glfwGetTime()), playerEntity(nullptr)
{

}

World::~World()
{

}

static glm::vec3 getRayDirection(int screenWidth, int screenHeight, double mouseX, double mouseY, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Convert mouse coordinates to NDC (Normalized Device Coordinates)
    //float ndcX = (2.0f * mouseX) / (static_cast<float>(screenWidth) - 1.0f);
    //float ndcY = 1.0f - (2.0f * mouseY) / static_cast<float>(screenHeight);
    float ndcX = 0.0f;
    float ndcY = 0.0f;
    // Create a ray in clip space
    glm::vec4 rayClip(ndcX, ndcY, 0.0f, 1.0f);

    glm::mat4 viewProjectionInverse = glm::inverse(projectionMatrix * viewMatrix);

    // Unproject the ray into view space
    glm::vec4 rayView = viewProjectionInverse * rayClip;
    rayView /= rayView.w;

    // The ray direction is the vector from the camera position to the unprojected point
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayView) - glm::vec3(viewMatrix[3]));

    return rayDirection;
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

            minPoint = glm::min(minPoint, center - objects[i]->extents * 0.5f);
            maxPoint = glm::max(maxPoint, center + objects[i]->extents * 0.5f);

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

Mesh* World::importFBX(std::string path)
{
    // Load FBX mesh and material data
    Mesh* mesh = new MeshFBX(path, true);

    if (mesh)
    {
        // Number of materials in the FBX
        unsigned int nMaterials = mesh->nMaterials;

        // Create a material for every unique material in loaded FBX
        for (unsigned int i = 0; i < nMaterials; i++)
        {
            // Create new material
            resource.createNewMaterial(mesh->matData[i].name, resource.shader("geometry_default"));

            Material* mat = resource.getMaterial(mesh->matData[i].name);

            // Set material parameters
            mat->vColor["diffuse"] = mesh->matData[i].diffuseColor; // Diffuse color
            mat->vColor["specular"] = mesh->matData[i].specularColor; // Specular color

            // Texture scale
            mat->vVec2["textureScale"] = Vec2Param(glm::vec2(5.0f, 5.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));

            // Shininess
            mat->vFloat["shininess"] = FloatParam(mesh->matData[i].shininess, 0.2f, 500.0f);

            // Reflectivity
            mat->vFloat["reflectivity"] = FloatParam(mesh->matData[i].shininess, 0.2f, 500.0f);

            // Normal strength
            mat->vFloat["normalStrength"] = FloatParam(20.1f, 0.0f, 60.5f);


            mat->hasDiffuseTexture = false;
            mat->vTexture["textureDiffuse"] = resource.getNullTexture();

            mat->hasNormalsTexture = false;
            mat->vTexture["textureNormal"] = resource.getNullTexture();

            mat->hasSpecularTexture = false;
            mat->vTexture["textureSpecular"] = resource.getNullTexture();


            // Attempt to load diffuse texture if one exists
            if (mesh->matData[i].hasDiffuse)
            {
                // Texture is not embedded, load from disk
                if (!mesh->matData[i].diffuseTexture.isEmbedded)
                {
                    mat->hasDiffuseTexture = true;
                    mat->vTexture["textureDiffuse"] = resource.getTexture(mesh->matData[i].diffuseTexture.texturePath);
                }
                else  //Texture is embedded, load from raw data
                {
                    mat->hasDiffuseTexture = true;

                    Texture* texture = new Texture();
                    texture->loadEmbedded(
                        mesh->matData[i].diffuseTexture.embeddedData.data,
                        mesh->matData[i].diffuseTexture.embeddedData.width,
                        mesh->matData[i].diffuseTexture.embeddedData.height
                    );

                    std::string name = mesh->matData[i].name + " embedded diffuse";
                    resource.addTexture(name, texture);

                    mat->vTexture["textureDiffuse"] = resource.getTexture(name);
                }
                
            }


            // Attempt to load normals texture if one exists
            if (mesh->matData[i].hasNormals)
            {
                // Texture is not embedded, load from disk
                if (!mesh->matData[i].normalTexture.isEmbedded)
                {
                    mat->hasNormalsTexture = true;
                    mat->vTexture["textureNormal"] = resource.getTexture(mesh->matData[i].normalTexture.texturePath);
                }
                else if (mesh->matData[i].normalTexture.isEmbedded) //Texture is embedded, load from raw data
                {
                    mat->hasDiffuseTexture = true;

                    Texture* texture = new Texture();
                    texture->loadEmbedded(
                        mesh->matData[i].normalTexture.embeddedData.data,
                        mesh->matData[i].normalTexture.embeddedData.width,
                        mesh->matData[i].normalTexture.embeddedData.height
                    );

                    std::string name = mesh->matData[i].name + " embedded normals";
                    resource.addTexture(name, texture);

                    mat->vTexture["textureNormal"] = resource.getTexture(name);
                }
                
            }

            // Attempt to load specular texture if one exists
            if (mesh->matData[i].hasSpecular)
            {
                // Texture is not embedded, load from disk
                if (!mesh->matData[i].specularTexture.isEmbedded)
                {
                    mat->hasSpecularTexture = true;
                    mat->vTexture["textureSpecular"] = resource.getTexture(mesh->matData[i].normalTexture.texturePath);
                }

            }
            
        }

        return mesh;
    }
    else
    {
        Log::error("Failed to import FBX!");

        return nullptr;
    }

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

    mouseWorldPos = { 0.0f, 0.0f, 0.0f };

    // Load all shaders
    resource.loadShader("geometry_default", "geo_pass.frag", "geo_pass.vert");
    resource.loadShader("lighting", "lighting.frag", "lighting.vert");
    resource.loadShader("point_shadows_default", "PointLightShadow.frag", "PointLightShadow.vert", "PointLightShadow.geom");
    resource.loadShader("post_process_default", "PostProcess.frag", "PostProcess.vert");
    resource.loadShader("particles_default", "particles.frag", "particles.vert");

    //  resource.loadShader("shadows_default", "shadow.frag", "shadow.vert");
    //resource.loadShader("skydome", "skydome.frag", "skydome.vert");
    
    Mesh* guitarMesh = importFBX("assets/mesh/monkey.fbx");
    Mesh* spheresMesh = importFBX("assets/mesh/sphere.fbx");
    Mesh* spriteMesh = importFBX("assets/mesh/sprite_zUp.fbx");
    Mesh* sponzaMesh = importFBX("assets/mesh/playground.fbx");
    Mesh* skyMesh = importFBX("assets/mesh/skySphere.fbx");

    
    Mesh* terrainMesh = new MeshTerrain(3000.0f, 128, 2.0f);
    MeshTerrain* tRef = reinterpret_cast<MeshTerrain*>(terrainMesh);
    tRef->loadHeightMap("assets/textures/heightmap2.jpg");
    tRef->applyHeightMap();
    

    // Set sky texture
    resource.setSkyTexture(resource.getTexture("assets/textures/hdri/kloofendal_43d_clear_2k.hdr"));

    // Create concrete material
    resource.createNewMaterial("mat_concrete", resource.shader("geometry_default"));

    // Create concrete material's parameters
    resource.getMaterial("mat_concrete")->vTexture["textureDiffuse"] = resource.getTexture("assets/textures/concrete_1.png");
    resource.getMaterial("mat_concrete")->vTexture["textureNormal"] = resource.getTexture("assets/textures/concrete_1_NRM.png");
    resource.getMaterial("mat_concrete")->vVec2["textureScale"] = Vec2Param(glm::vec2(5.0f, 5.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
    resource.getMaterial("mat_concrete")->vFloat["shininess"] = FloatParam(120.0f, 0.2f, 500.0f);
    resource.getMaterial("mat_concrete")->vFloat["normalStrength"] = FloatParam(48.0f, 0.0f, 60.5f);
    resource.getMaterial("mat_concrete")->vColor["diffuse"] = Color4(1.0f, 1.0f, 1.0f, 1.0f); // Diffuse color
    resource.getMaterial("mat_concrete")->vColor["specular"] = Color4(1.0f, 1.0f, 1.0f, 1.0f); // Specular color
    resource.getMaterial("mat_concrete")->hasDiffuseTexture = true;
    resource.getMaterial("mat_concrete")->hasNormalsTexture = true;


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
    plTrn2->pos = { 0.0f, 4.0f, 6.0f };
    plTrn2->scl = { 1.0f, 1.0f, 1.0f };

    // Create point light 2
    PointLight* light2 = new PointLight("Point light 2", 1024, 1024);

    light2->addComponent(plComp2);
    light2->addComponent(plTrn2);

    pointLights.push_back(light2);
    

    // Create point light component 2
    PointLightComponent* plComp1 = new PointLightComponent();
    plComp1->color = { 0.8f, 0.7f, 0.4f };
    plComp1->resolution = { 1024, 1024 };
    plComp1->strength = 60.0f;
    plComp1->falloff = 2.0f;
    plComp1->disableShadows = false;
    plComp1->farPlane = 180.0f;

    // Create point light transform component 2
    TransformComponent* plTrn1 = new TransformComponent();
    plTrn1->pos = { -4.0f, -13.0f, 6.0f };
    plTrn1->scl = { 1.0f, 1.0f, 1.0f };

    // Create point light 2
    PointLight* light1 = new PointLight("Point light 1", 1024, 1024);

    light1->addComponent(plComp1);
    light1->addComponent(plTrn1);

    pointLights.push_back(light1);




    // Create plane entity
    Entity* terrainEntity = new Entity("Terrain");

    // Transform for sponza entity
    TransformComponent* trTerrain = new TransformComponent();
    trTerrain->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    //trTerrain->scl = glm::vec3(0.1f, 0.1f, 0.1f);
    trTerrain->scl = glm::vec3(1.0f, 1.0f, 1.0f);
    // Renderer for plane entity
    RenderComponent* rndrSponzaTerrain = new RenderComponent();
    rndrSponzaTerrain->mesh = terrainMesh;
    rndrSponzaTerrain->setMaterial(resource.getMaterial("mat_concrete"), terrainEntity, 0);
    //rndrSponzaTerrain->setMaterialsFromMesh(&resource);

    
    // Terrain component
    TerrainComponent* tComp = new TerrainComponent();
    tComp->size = 3000.0f;
    tComp->height = 1.0f;
    tComp->resolution = 128;

    // Add components to plane entity
    terrainEntity->addComponent(trTerrain);
    terrainEntity->addComponent(rndrSponzaTerrain);
    terrainEntity->addComponent(tComp);
    
    
   // terrainEntity->addComponent(trTerrain);
    //terrainEntity->addComponent(rndrSponzaTerrain);
    
    entities.push_back(terrainEntity);
    

    // Particle system test
    resource.createNewMaterial("mat_sprite", resource.shader("particles_default"));
    resource.getMaterial("mat_sprite")->vTexture["sprite_texture"] = resource.getTexture("assets/textures/sprite.png");

    TransformComponent* trPart = new TransformComponent();
    trPart->pos = glm::vec3(0.0f, 0.0f, 4.5f);
    trPart->scl = glm::vec3(1.0f, 1.0f, 1.0f);

    RenderComponent* rndrPart= new RenderComponent();
    rndrPart->mesh = spheresMesh;
    
    ParticleEmitter* pSystem = new ParticleEmitter("particle system", 3, 100);

    rndrPart->setMaterial(resource.getMaterial("mat_concrete"), pSystem, 0);

    pSystem->addComponent(trPart);
    pSystem->addComponent(rndrPart);

    pSystem->init();

    particles.push_back(pSystem);
    
    //Serialization::Serialize(resource.getMaterial("mat_concrete"), ObjectType::MATERIAL);
    


    
    
    // Create skybox entity
    Entity* skyBox = new Entity("sky dome");

    // Transform for skybox entity
    TransformComponent* trSky = new TransformComponent();
    trSky->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    trSky->scl = glm::vec3(50.0f, 50.0f, 50.0f);

    // Renderer for skybox entity
    RenderComponent* rndrSky = new RenderComponent();
    rndrSky->mesh = skyMesh;
    rndrSky->isSky = true;
    rndrSky->setMaterial(resource.getMaterial("mat_concrete"), skyBox, 0);

    // Add components to skybox entity
    skyBox->addComponent(rndrSky);
    skyBox->addComponent(trSky);

    // Add skybox to world
    entities.push_back(skyBox);
    



    // Monkey entity
    
    
    Entity* monk = new Entity("monkey");

    TransformComponent* trMonk = new TransformComponent();
    trMonk->pos = glm::vec3(0.0f, 13.0f, 6.5f);
    trMonk->scl = glm::vec3(2.0f, 2.0f, 2.0f);

    RenderComponent* rndrMonk = new RenderComponent();
    rndrMonk->mesh = guitarMesh;

    rndrMonk->setMaterialsFromMesh(&resource);

   //rndrMonk->setMaterial(resource.getMaterial("mat_concrete"), monk, 0);
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


    playerEntity = player;


    //createBvhObjects();

    std::cout << "Number of triangle AABB's: " << objList.size() << "\n\n";

    if (objList.size() > 0)
    {
        Log::info("Creating BVH tree...");
        //bvh = createBVH(objList, 0);
    }

    //std::cout << "max leaf depth: " << maxLeafDepth << "\nmin leaf depth: " << minLeafDepth << "\n\n";

}

void World::updateTransforms(glm::vec3 eye, glm::vec3 center, float tilt, float spin)
{
    glm::mat4 viewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 0.0f, 1.0f));
    worldView = viewMatrix;

    //worldView = Rotate(0, tilt - 90) * Rotate(2, spin) * Translate(-eye[0], -eye[1], -eye[2]);
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




void World::createBvhObjects()
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

        // Skip if its the sky
        if (renderComp->isSky)
            continue;

        Mesh* mesh = renderComp->mesh;


        glm::mat4 modelTr = glm::mat4(1.0f);
        modelTr = glm::translate(modelTr, transformComp->pos);
        modelTr = glm::rotate(modelTr, (transformComp->angle), transformComp->rot);
        modelTr = glm::scale(modelTr, transformComp->scl * glm::vec3(0.01f, 0.01f, 0.01f));

       
        // For every sub-mesh on mesh
        for (unsigned int i = 0; i < mesh->nMeshes; i++)
        {
            std::vector<Vertex>& vertices = mesh->meshData[i].vertices;
            std::vector<unsigned int>& indices = mesh->meshData[i].indices;

            // For triangle on mesh
            for (unsigned int j = 0; j < mesh->meshData[i].indices.size(); j+=3)
            {
                glm::vec4 minPoint = { 9999.0f, 9999.0f, 9999.0f, 1.0f };
                glm::vec4 maxPoint = { -9999.0f, -9999.0f, -9999.0f, 1.0f };
                glm::vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
                glm::vec3 dim = { 0.0f, 0.0f, 0.0f };

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->meshData[i].EBO);

                unsigned int index0 = mesh->meshData[i].indices[j];
                unsigned int index1 = mesh->meshData[i].indices[j+1];
                unsigned int index2 = mesh->meshData[i].indices[j+2];

                // Triangle vertices 
                glm::vec3 v0 = mesh->meshData[i].vertices[index0].position;
                glm::vec3 v1 = mesh->meshData[i].vertices[index1].position;
                glm::vec3 v2 = mesh->meshData[i].vertices[index2].position;

                // Triangle vertices in world space
                glm::vec4 A = modelTr * glm::vec4(v0, 1.0f);
                glm::vec4 B = modelTr * glm::vec4(v1, 1.0f);
                glm::vec4 C = modelTr * glm::vec4(v2, 1.0f);

                // Get min point
                minPoint = glm::min(A, minPoint);
                minPoint = glm::min(B, minPoint);
                minPoint = glm::min(C, minPoint);

                // Get max point
                maxPoint = glm::max(A, maxPoint);
                maxPoint = glm::max(B, maxPoint);
                maxPoint = glm::max(C, maxPoint);

                // Set dimensions
                dim.x = glm::abs(maxPoint.x - minPoint.x);
                dim.y = glm::abs(maxPoint.y - minPoint.y);
                dim.z = glm::abs(maxPoint.z - minPoint.z);

                // Set center point
                centerPoint = glm::vec3(maxPoint) - dim * 0.5f;


                // Create new AABB
                Box3D* box = new Box3D(centerPoint, dim * 0.5f, reinterpret_cast<void*>(e));

                // Add to list of triangle AABB's
                objList.push_back(box);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

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


bool World::testRayAgainstNode(Ray3D ray, TreeNode* node, int depth, glm::vec3* hitPos)
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

                    *hitPos = box->center;

                    return true;
                }
                else if (node->type == NodeType::INTERNAL)
                {
                    bool hitLeft = testRayAgainstNode(ray, node->left, depth++, hitPos);
                    bool hitRight = testRayAgainstNode(ray, node->right, depth, hitPos);

                    return hitLeft || hitRight;
                }
            }
            else
            {
                return false;
            }

        }
        else
            return false;
    }
    return false;
}

void World::createAABBComponents()
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
        glm::vec3 maxPoint = { -9999.0f, -9999.0f, -9999.0f };
        glm::vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
        glm::vec3 dim = { 0.0f, 0.0f, 0.0f };

        //std::vector<std::vector<Vertex>>& vertices = renderComp->mesh->vertices;
        //std::vector < std::vector<unsigned int>>& indices = renderComp->mesh->indices;

        std::vector<std::vector<Vertex>> vertices;
        std::vector < std::vector<unsigned int>> indices;

        for (int i = 0; i < vertices.size(); i++)
        {
            for (int j = 0; j < vertices[i].size(); j++)
            {
                glm::vec3& p = vertices[i][j].position;
                glm::vec4 p4 = modelTr * glm::vec4(p.x, p.y, p.z, 1.0f);

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