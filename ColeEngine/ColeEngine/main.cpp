
//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"

#include "UI.h"

#define STB_IMAGE_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <iostream>

#include "Component.h"
#include "Entity.h"
#include "Transform.h"
#include "Platform.h"
#include "Shapes.h"
#include "Shader.h"
#include "World.h"
#include "Engine.h"

#include "PlayerSystem.h"
#include "RenderSystem.h"
#include "WorldEditSystem.h"

#include "UI.h"
#include "ResourceManager.h"
#include "Logging.h"

//#define ASSIMP_USE_HUNTER




int main()
{
    Log::info("Starting the engine!");

    // Make a window for everything to live in
    Platform platform = Platform(2200, 1300);
   
    // Create the resource manager
    ResourceManager resourceManager;

    // Create world and populate it with entities
    World world(platform, resourceManager);
    world.initWorld();

    // Main object to reference
    Engine engine(platform, world, resourceManager);

    // Start up imgui
    UI ui(engine);


    // -------- SYSTEMS --------- \\

    // Handles graphics/rendering. 
    RenderSystem renderSystem(engine);

    // Handles player movement 
    PlayerSystem playerSystem(engine);
    
    //WorldEditSystem worldEditSystem(engine);
    

    while (!glfwWindowShouldClose(platform.window)) 
    {
        glfwPollEvents();

        // Update platform. (input/window handling)
        platform.update();

        // Update world
        world.update();

        // Update player system
        playerSystem.update(engine);

        // Update render system (draws renderable entities)
        renderSystem.update(engine);
        
        //worldEditSystem.update(engine);

        // Update Imgui stuff
        ui.draw();
        
        glfwSwapBuffers(platform.window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
   
    glfwTerminate();

	return 0;
}