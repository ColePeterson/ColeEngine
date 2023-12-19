#include "Engine.h"

#include<vector>



Engine::Engine(Platform& _platform, World& _world, ResourceManager& _resource)
	: platform(_platform), onSelect(false), world(_world), selectedEntity(nullptr), resource(_resource), mode(EngineMode::EDITOR)
{
	




}





