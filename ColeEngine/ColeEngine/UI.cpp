#include "UI.h"
#include "PointLight.h"


UI::UI(Engine& _engine)
    : engine(_engine)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(engine.getPlatform().window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::GetStyle().ScaleAllSizes(1.8f);
    ImGui::GetIO().FontGlobalScale = 1.8f;
}


static void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
}

// Window for scene specific parameters
void UI::sceneLighting_draw()
{
    if (ImGui::Begin("Scene Lighting"))
    {
        ImGui::Text(" %.1f FPS;  %.3f ms/frame;", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        ImGui::NewLine();

        ImGui::SliderFloat("Light X: ", &engine.getWorld().lightPos.x, -70.0f, 70.0f);
        ImGui::SliderFloat("Light Y: ", &engine.getWorld().lightPos.y, -70.0f, 70.0f);
        ImGui::SliderFloat("Light Z: ", &engine.getWorld().lightPos.z, -70.0f, 70.0f);

        ImGui::Checkbox("Draw debug lines", &engine.debug.drawDebugLines);

        ImGui::ColorEdit4("Ambient Color", sceneLighting.ambientColor);

        ImGui::Checkbox("Basic shading", &engine.debug.basicShading);

        ImGui::SliderFloat("Shader debug float 1: ", &engine.debug.float1, 1.0f, 10.0f);
        ImGui::SliderFloat("Shader debug float 2: ", &engine.debug.float2, 1.0f, 1000.0f);

        engine.getWorld().ambientColor.x = sceneLighting.ambientColor[0];
        engine.getWorld().ambientColor.y = sceneLighting.ambientColor[1];
        engine.getWorld().ambientColor.z = sceneLighting.ambientColor[2];

        ImGui::End();

    }
}

// Window for scene browser. Select entities
void UI::sceneBrowser_draw()
{
    if (ImGui::Begin("Scene Browser"))
    {
        engine.onSelect = false;

        // List of all entities in world
        if (ImGui::BeginListBox(" "))
        {
            unsigned int nEntities = engine.getWorld().entities.size();
            std::vector<Entity*>& entities = engine.getWorld().entities;

            // For every entity in world
            for (unsigned int i = 0; i < nEntities; i++)
            {
                const bool is_selected = (sceneBrowser.selected == i);

                // Select new selected entity
                if (ImGui::Selectable(entities[i]->getName().c_str(), is_selected))
                {
                    sceneBrowser.selected = i;
                    engine.selectedEntity = entities[i];
                    engine.onSelect = true;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }

            }

            unsigned int nLights = engine.getWorld().pointLights.size();
            std::vector<PointLight*>& lights = engine.getWorld().pointLights;

            // For every point light in world
            for (unsigned int i = 0; i < nLights; i++)
            {
                const bool is_selected = (sceneBrowser.selected == i + nEntities);

                // Select new selected entity
                if (ImGui::Selectable(lights[i]->getName().c_str(), is_selected))
                {
                    sceneBrowser.selected = i + nEntities;
                    engine.selectedEntity = reinterpret_cast<PointLight*>(lights[i]);
                    engine.onSelect = true;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }

        ImGui::End();
    }
}

// Edit selected transform component
void UI::transformComp_draw()
{
    TextCentered("Transform Component");

    ImGui::NewLine();

    TransformComponent* transform = engine.selectedEntity->getComponent<TransformComponent>();

    ImGui::Text("Position");
    if (ImGui::SliderFloat("pos X: ", &transform->pos.x, -100.0f, 100.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("pos Y: ", &transform->pos.y, -100.0f, 100.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("pos Z: ", &transform->pos.z, -100.0f, 100.0f)) engine.onSelect = true;

    ImGui::Text("Rotation");
    if (ImGui::SliderFloat("rot X: ", &transform->rot.x, 0.0f, 1.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("rot Y: ", &transform->rot.y, 0.0f, 1.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("rot Z: ", &transform->rot.z, 0.0f, 1.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("angle: ", &transform->angle, -360.0f, 360.0f)) engine.onSelect = true;

    ImGui::Text("Scale");
    if (ImGui::SliderFloat("scale X: ", &transform->scl.x, -10.0f, 10.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("scale Y: ", &transform->scl.y, -10.0f, 10.0f)) engine.onSelect = true;
    if (ImGui::SliderFloat("scale Z: ", &transform->scl.z, -10.0f, 10.0f)) engine.onSelect = true;

    ImGui::NewLine();
}


void UI::pointLightComp_draw()
{
    TextCentered("Point Light Component");
    ImGui::NewLine();

    PointLightComponent* pnt = engine.selectedEntity->getComponent<PointLightComponent>();
    
    ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&pnt->color));

    ImGui::NewLine();

    ImGui::SliderFloat("Strength", &pnt->strength, 0.0f, 200.0f);

    ImGui::NewLine();

    ImGui::SliderFloat("Falloff", &pnt->falloff, 0.1f, 2.0f);

    ImGui::NewLine();

    ImGui::SliderFloat("Far Plane", &pnt->farPlane, 10.1f, 200.0f);

}

// Edit selected render component
void UI::renderComp_draw()
{
    TextCentered("Render Component");
    ImGui::NewLine();

    RenderComponent* render = engine.selectedEntity->getComponent<RenderComponent>();
    Material* mat = render->material;

    // Drop down for material selection of current selected entity
    if (ImGui::BeginCombo("Material Select", selectedMaterialName.c_str()))
    {
        for (auto& it : engine.Resource().materials)
        {
            bool is_selected = true;

            if (ImGui::Selectable(it.first.c_str(), is_selected))
            {
                if (it.second)
                {
                    render->setMaterial(it.second, engine.selectedEntity);
                    selectedMaterialName = it.first;
                    render = engine.selectedEntity->getComponent<RenderComponent>();
                    mat = render->material;
                }
            }
        }
        ImGui::EndCombo();
    }


    // Edit selected material
    ImGui::NewLine();
    TextCentered("Current Material: " + selectedMaterialName);

    if (mat)
    {
        // Edit color parameters
        for (auto& it : mat->vColor)
        {
            ImGui::ColorEdit3(it.first.c_str(), reinterpret_cast<float*>(&it.second));
        }

        ImGui::NewLine();

        // Edit vec3 parameters
        for (auto& it : mat->vVec3)
        {
            ImGui::Text(it.first.c_str());

            ImGui::SliderFloat("X: ", &it.second.val.x, it.second.min.x, it.second.max.x);
            ImGui::SliderFloat("Y: ", &it.second.val.y, it.second.min.y, it.second.max.y);
            ImGui::SliderFloat("Z: ", &it.second.val.z, it.second.min.z, it.second.max.z);
        }

        ImGui::NewLine();

        // Edit vec2 parameters
        for (auto& it : mat->vVec2)
        {
            ImGui::Text(it.first.c_str());

            ImGui::SliderFloat("X: ", &it.second.val.x, it.second.min.x, it.second.max.x);
            ImGui::SliderFloat("Y: ", &it.second.val.y, it.second.min.y, it.second.max.y);
        }

        ImGui::NewLine();

        // Edit float parameters
        for (auto& it : mat->vFloat)
        {
            ImGui::Text(it.first.c_str());

            std::string label = it.first;
            ImGui::SliderFloat(label.c_str(), &it.second.val, it.second.min, it.second.max);
        }


        // Edit texture slots
        ImGui::NewLine();
        ImGui::Text("Texture Slots");
        ImGui::NewLine();

        for (auto& it : mat->vTexture)
        {
            ImGui::Text(it.first.c_str());

            if (ImGui::BeginCombo(it.first.c_str(), selectedTextureName.c_str()))
            {
                for (auto& it2 : engine.Resource().textures)
                {
                    bool is_selected = true;

                    if (ImGui::Selectable(it2.first.c_str(), is_selected))
                    {
                        it.second = it2.second;
                        selectedTextureName = it2.first;
                    }

                }
                ImGui::EndCombo();
            }
        }
    }
}

// Show all components of selected entity
void UI::componentTab_draw()
{
    if (ImGui::Begin("Components"))
    {
        if (ImGui::BeginListBox(" ", ImVec2(390, 600)))
        {
            if (engine.selectedEntity)
            {
                TransformComponent* transform = nullptr;
                RenderComponent* render = nullptr;
                for (auto& it : engine.selectedEntity->components)
                {
                    switch (it.first)
                    {
                    case ComponentType::TransformComponent:
                        transformComp_draw();
                        break;

                    case ComponentType::RenderComponent:
                        renderComp_draw();
                        break;

                    case ComponentType::PointLightComponent:
                        pointLightComp_draw();
                        break;
                    }
                }
            }

            ImGui::EndListBox();
        }
        ImGui::End();
    }
}


// Draw all windows
void UI::draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    
    sceneLighting_draw();
    sceneBrowser_draw();
    componentTab_draw();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
