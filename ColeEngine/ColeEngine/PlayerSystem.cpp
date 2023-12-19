#include "PlayerSystem.h"

#include "Entity.h"
#include "Component.h"
#include <vector>


float RAD = PI / 180.0f;


PlayerSystem::PlayerSystem(Engine& _engine)
{

}

void PlayerSystem::update(Engine& engine)
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
                dist = engine.getWorld().time_dx* playerComp->runSpeed;
            else
                dist = engine.getWorld().time_dx* playerComp->walkSpeed;


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

            Ray3D ray = Ray3D(playerComp->eye, dir);

            engine.getWorld().minDepth = 9999.0f;


            // Collison detection
            //engine.getWorld().testRayAgainstNode(ray, engine.getWorld().bvh);

            if (engine.getWorld().minDepth > dist)
            {

                playerComp->eye += dist * dir;

                transformComp->pos = playerComp->eye + glm::vec3(0.0f, 0.0f, -1.0f);
                transformComp->transform = glm::scale(transformComp->transform, transformComp->scl);
                transformComp->transform = glm::translate(transformComp->transform, transformComp->pos);
            }

            engine.getWorld().updateTransforms(playerComp->eye, playerComp->tilt, playerComp->spin);

		}

	}

}
