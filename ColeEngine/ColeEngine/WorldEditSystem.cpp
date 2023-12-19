#include "WorldEditSystem.h"

#include "geomlib.h"


WorldEditSystem::WorldEditSystem(Engine& _engine)
{

}


// Function to convert screen coordinates to normalized device coordinates (NDC)
static glm::vec3 screenToNDC(double xpos, double ypos, int screenWidth, int screenHeight)
{
    float x = (2.0f * xpos) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * ypos) / screenHeight;
    float z = 1.0f;

   
    return glm::vec3(x, y, z);
}

// Function to convert NDC to world coordinates
static glm::vec3 ndcToWorld(const glm::vec3& ndc, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    glm::mat4 invViewProj = glm::inverse(projectionMatrix * viewMatrix);
    glm::vec4 worldCoords = invViewProj * glm::vec4(ndc, 1.0f);

    return glm::vec3(worldCoords) / worldCoords.w;
}



void WorldEditSystem::mouseSelectEntity(Engine& engine)
{

    Platform& platform = engine.getPlatform();

    double mouseX = platform.mouseX;
    double mouseY = platform.mouseX;

    glm::mat4 viewMatrix, projectionMatrix;
    // Obtain your view and projection matrices from your camera setup

    // Convert screen coordinates to NDC
    glm::vec3 ndc = screenToNDC(mouseX, mouseY, platform.width, platform.height);

    // Convert NDC to world coordinates
    glm::vec3 rayOrigin = engine.getWorld().eyePos;  // Camera position in world coordinates
    glm::vec3 rayDirection = glm::normalize(ndcToWorld(ndc, viewMatrix, projectionMatrix) - rayOrigin);

    //std::cout << "Ray Origin: (" << rayOrigin.x << ", " << rayOrigin.y << ", " << rayOrigin.z << ")" << std::endl;
    //std::cout << "Ray Direction: (" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << ")" << std::endl;

    std::vector<Entity*> & entities = engine.getWorld().entities;
    for (Entity* e : entities)
    {
        AABBComponent* aabb = e->getComponent<AABBComponent>();

        if (aabb)
        {
            Ray3D ray = Ray3D(rayOrigin, rayDirection);
            Box3D* box = aabb->box;

            //std::cout << "pos x: " << box->center.x << "\npos y: " << box->center.y << "\npos z: " << box->center.z << "\n\n";
            //std::cout << "dim x: " << box->extents.x << "\ndim y: " << box->extents.y << "\ndim z: " << box->extents.z << "\n\n";
            float rt = 0.0f;
            if (Intersects(ray, *box, &rt))
            {
                std::cout << "Selected " << e->getName() << "\n\n";
                engine.selectedEntity = e;
                engine.onSelect = true;
            }
            else
            {
                //engine.selectedEntity = nullptr;
            }
        }

    }
}

void WorldEditSystem::update(Engine& engine)
{
	Platform& platform = engine.getPlatform();

    if (platform.mouseLeftRelease)
    {
        mouseSelectEntity(engine);
    }

}