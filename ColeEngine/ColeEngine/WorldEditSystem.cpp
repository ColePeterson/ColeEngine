#include "WorldEditSystem.h"

#include "geomlib.h"


WorldEditSystem::WorldEditSystem(Engine& _engine)
{

}



void WorldEditSystem::getMouseWorldPosition(Engine& engine)
{

    
    Platform& platform = engine.getPlatform();

    double mouseX = platform.mouseX;
    double mouseY = platform.mouseY;

    
    
   
}

void WorldEditSystem::update(Engine& engine)
{
	
    // On selection of new entity
    if (engine.onSelect)
    {
        TerrainComponent* terrain = engine.selectedEntity->getComponent<TerrainComponent>();

        // Check if entity is a terrain object
        if (terrain)
        {
            engine.mode = EngineMode::TERRAIN;
        }
        else
        {
            engine.mode = EngineMode::EDITOR;
        }
    }
}