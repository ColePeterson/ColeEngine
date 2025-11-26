#include "PlayerSystem.h"

#include "Entity.h"
#include "Component.h"
#include <vector>


float RAD = PI / 180.0f;


PlayerSystem::PlayerSystem(Engine& _engine)
{

}

void PlayerSystem::flyCam(Engine& engine)
{
    Platform& platform = engine.getPlatform();

    std::vector<Entity*>& entities = engine.getWorld().entities;

    for (Entity* e : engine.getWorld().entities)
    {
        PlayerComponent* playerComp = e->getComponent<PlayerComponent>();

        if (playerComp)
        {
            TransformComponent* transformComp = e->getComponent<TransformComponent>();

            if (platform.mouseRight)
            {
                playerComp->spin += platform.dx * 0.33;
                playerComp->tilt += platform.dy * 0.33;
            }

            float dist = 0.0f;

            if (platform.lshift_down)
                dist = engine.getWorld().time_dx * playerComp->runSpeed;
            else
                dist = engine.getWorld().time_dx * playerComp->walkSpeed;


            glm::vec3 dir = glm::vec3(0.0f, 0.0f, 0.0f);

            if (platform.w_down)
            {
                dir += glm::vec3(sin(playerComp->spin * RAD), cos(playerComp->spin * RAD), -sin(playerComp->tilt * RAD));
            }
            if (platform.s_down)
            {
                dir += -glm::vec3(sin(playerComp->spin * RAD), cos(playerComp->spin * RAD), -sin(playerComp->tilt * RAD));
            }
            if (platform.d_down)
            {
                dir += glm::vec3(cos(playerComp->spin * RAD), -sin(playerComp->spin * RAD), 0.0f);
            }
            if (platform.a_down)
            {
                dir += -glm::vec3(cos(playerComp->spin * RAD), -sin(playerComp->spin * RAD), 0.0f);
            }

            glm::vec3 view = { cosf(glm::radians(playerComp->spin)), sinf(glm::radians(playerComp->spin)), 1.0f };
            view = glm::normalize(view);
            //Ray3D ray = { playerComp->eye, view };

            Ray3D ray = Ray3D(playerComp->eye, dir);



            engine.getWorld().minDepth = 99999.0f;

            playerComp->eye += dist * dir;

            transformComp->pos = playerComp->eye + glm::vec3(0.0f, 0.0f, -1.0f);
            transformComp->transform = glm::scale(transformComp->transform, transformComp->scl);
            transformComp->transform = glm::translate(transformComp->transform, transformComp->pos);

            /*
            // Collison detection

            glm::vec3 hitPos = { 0.0f, 0.0f, 0.0f };

            bool intersect = engine.getWorld().testRayAgainstNode(ray, engine.getWorld().bvh, 0, &hitPos);

            //engine.getWorld().dist = dist * 100.0f;

            if (engine.getWorld().minDepth > dist * 2)
            {
                playerComp->eye += dist * dir;

                transformComp->pos = playerComp->eye + glm::vec3(0.0f, 0.0f, -1.0f);
                transformComp->transform = glm::scale(transformComp->transform, transformComp->scl);
                transformComp->transform = glm::translate(transformComp->transform, transformComp->pos);
            }
            */

            engine.getWorld().updateTransforms(playerComp->eye, playerComp->center, playerComp->tilt, playerComp->spin);

        }

    }
}


// Controls the editor (orbit around selection) camera
void PlayerSystem::editorCam(Engine& engine)
{
    Platform& platform = engine.getPlatform();

    // Player is in scene
    if (engine.getWorld().playerEntity)
    {
       
        Entity* player = engine.getWorld().playerEntity;

        PlayerComponent* pc = player->getComponent<PlayerComponent>();
        TransformComponent* tc = player->getComponent<TransformComponent>();


        // Mouse movement
        if (platform.mouseRight)
        {
            pc->spin += platform.dx * pc->orbitSpeed;
            pc->tilt += platform.dy * pc->orbitSpeed;

            // Clamp vertical angle to avoid camera flipping
            pc->tilt = glm::clamp(pc->tilt, -1.5f, 1.5f);
        }


        // Pan camera with middle click
        if (platform.middleDown)
        {
            glm::vec3 right = glm::normalize(glm::cross(pc->center - pc->eye, glm::vec3(0.0f, 0.0f, 1.0f)));
            glm::vec3 upVector = glm::normalize(glm::cross(right, pc->center - pc->eye));

            pc->center += right * pc->panSpeed * static_cast<float>(-platform.dx) + 
                upVector * pc->panSpeed * static_cast<float>(platform.dy);
            pc->eye += right * pc->panSpeed * static_cast<float>(-platform.dx)
                + upVector * pc->panSpeed * static_cast<float>(platform.dy);
        }
        


        // Set orbit look at position to position of selected entity when F is pressed
        if (engine.selectedEntity && platform.f_down)
        {
            TransformComponent* tcs = engine.selectedEntity->getComponent<TransformComponent>();

            if (tcs)
                pc->center = tcs->pos;
        }

        // Adjust orbit distance
        pc->orbitDist -= platform.scroll_dx * pc->zoomSpeed;

        // Orbit position
        glm::vec3 cameraPosition
        (
            pc->center.x + pc->orbitDist * glm::cos(pc->tilt) * glm::sin(pc->spin),
            pc->center.y + pc->orbitDist * glm::cos(pc->tilt) * glm::cos(pc->spin),
            pc->center.z + pc->orbitDist * glm::sin(pc->tilt)
        );
       

        // Adjust camera transforms
        pc->eye = cameraPosition;
        tc->pos = pc->eye;
        tc->transform = glm::scale(tc->transform, tc->scl);
        tc->transform = glm::translate(tc->transform, tc->pos);
    

        // Update world transforms
        engine.getWorld().updateTransforms(pc->eye, pc->center, pc->tilt, pc->spin);

        platform.scroll_dx = 0.0f;
    }
}

void PlayerSystem::update(Engine& engine)
{

    //flyCam(engine);

    editorCam(engine);
}
