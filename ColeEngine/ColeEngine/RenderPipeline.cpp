#include "RenderPipeline.h"
#include "PointLight.h"

#include "Mesh.h"

RenderPipeline::RenderPipeline(std::string _name, ShaderProgram* _shader)
    : name(_name), shader(_shader)
{

}


void RenderPipeline::draw(RenderComponent* render)
{
    if (render->mesh)
    {
        render->mesh->drawVAO();
    }
}


void RenderPipeline::setGeometryPassUnis(Engine& engine, GeometryPassUniforms uniforms)
{
    // World specific uniforms
    int loc = glGetUniformLocation(shader->programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.worldProj));

    loc = glGetUniformLocation(shader->programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.worldView));

    loc = glGetUniformLocation(shader->programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.worldInverse));

    loc = glGetUniformLocation(shader->programId, "time");
    glUniform1f(loc, engine.getWorld().time);

}


void RenderPipeline::setLightingPassUnis(Engine& engine, GBuffer* gBuffer, LightingPassUniforms uniforms)
{

    int loc = glGetUniformLocation(shader->programId, "viewPos");
    glUniform3fv(loc, 1, &engine.getWorld().eyePos[0]);


    // Bind G-Buffer textures to sampler slots
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getPosition()->get());
    loc = glGetUniformLocation(shader->programId, "gPosition");
    glUniform1i(loc, 0);

    // Set normals texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getNormal()->get());
    loc = glGetUniformLocation(shader->programId, "gNormal");
    glUniform1i(loc, 1);

    // Set albedo texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getAlbedo()->get());
    loc = glGetUniformLocation(shader->programId, "gAlbedo");
    glUniform1i(loc, 2);

    // Set specular texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getSpecular()->get());
    loc = glGetUniformLocation(shader->programId, "gSpecular");
    glUniform1i(loc, 3);


    // Set sky texture
    if (engine.Resource().getSkyTexture())
    {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, engine.Resource().getSkyTexture()->get());
        loc = glGetUniformLocation(shader->programId, "skyTexture");
        glUniform1i(loc, 4);
    }


    // Set world ambient color
    loc = glGetUniformLocation(shader->programId, "ambientColor");
    glUniform3fv(loc, 1, &engine.getWorld().ambientColor[0]);

    // Set world sun position
    loc = glGetUniformLocation(shader->programId, "sunPos");
    glUniform3fv(loc, 1, &engine.getWorld().lightPos[0]);

    // Set shading mode
    loc = glGetUniformLocation(shader->programId, "basicShading");
    glUniform1i(loc, engine.debug.basicShading);

    // Get all point lights in scene
    std::vector<PointLight*> pointLights(engine.getWorld().pointLights);

    

    // Set point light uniforms for lighting shader
    unsigned int nLights = pointLights.size();
    for (unsigned int i = 0; i < nLights; i++)
    {
        // Point lightis valid
        if (pointLights[i])
        {
            PointLight* light = engine.getWorld().pointLights[i];
            unsigned int textureID = light->getShadowFBO()->textureCube->get();

            TransformComponent* trn = pointLights[i]->getComponent<TransformComponent>();
            PointLightComponent* plc = pointLights[i]->getComponent<PointLightComponent>();

            // Components exist
            if (trn && plc)
            {
                // Set light cubemap sampler
                glActiveTexture(GL_TEXTURE5 + i);
                glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
                std::string uniName = "shadowMapPoint[" + std::to_string(i) + "]";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform1i(loc, 5 + i);

                // Set light position
                uniName = "lights[" + std::to_string(i) + "].pos";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform3fv(loc, 1, &trn->pos[0]);

                // Set light color
                uniName = "lights[" + std::to_string(i) + "].color";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform3fv(loc, 1, &plc->color[0]);

                // Set light strength
                uniName = "lights[" + std::to_string(i) + "].strength";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform1fv(loc, 1, &plc->strength);

                // Set light falloff
                uniName = "lights[" + std::to_string(i) + "].falloff";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform1fv(loc, 1, &plc->falloff);

                // Set light falloff
                uniName = "lights[" + std::to_string(i) + "].farPlane";
                loc = glGetUniformLocation(shader->programId, uniName.c_str());
                glUniform1fv(loc, 1, &plc->farPlane);

            }
            
        }
    }
}

void RenderPipeline::setShadowPassUnis(Engine& engine, RenderComponent* render, ShadowPassUniforms uniforms)
{

    int loc = glGetUniformLocation(shader->programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.lightProj));

    loc = glGetUniformLocation(shader->programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.lightView));

    loc = glGetUniformLocation(shader->programId, "lightPos");
    glUniform3fv(loc, 1, &(engine.getWorld().lightPos[0]));

    loc = glGetUniformLocation(shader->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.transform));

   
}

void RenderPipeline::setPointShadowPassUnis(Engine& engine, PointLightShadowPassUniforms pointPassUnis)
{
    int loc = glGetUniformLocation(shader->programId, "far_plane");
    glUniform1f(loc, pointPassUnis.farPlane);

    loc = glGetUniformLocation(shader->programId, "lightPos");
    glUniform3fv(loc, 1, &pointPassUnis.pointLightPos[0]);

    for (unsigned int i = 0; i < 6; ++i)
    {
        std::string uniName = "shadowMatrices[" + std::to_string(i) + "]";
        loc = glGetUniformLocation(shader->programId, uniName.c_str());
        //glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(uniforms.shadowTransforms[i]));
        glm::mat4 trn = pointPassUnis.shadowTransforms[i];
        glUniformMatrix4fv(loc, 1, GL_FALSE, &trn[0][0]);
    }
    
}

void RenderPipeline::setMaterialUniforms(Engine& engine, Material* mat)
{
    if (mat)
    {
        // Shader attached to material
        ShaderProgram* shader = mat->getShader();

        int loc = -1;
        // Set texture sampler uniforms
        int i = 0;
        for (auto& it : mat->vTexture)
        {
            std::string name = it.first;
            Texture* texture = it.second;

            int loc = glGetUniformLocation(shader->programId, name.c_str());
            glUniform1i(loc, i);

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture->get());

            i++;
        }

        // Set float uniforms
        for (auto& it : mat->vFloat)
        {
            std::string name = it.first;
            float value = it.second.val;

            int loc = glGetUniformLocation(shader->programId, name.c_str());
            glUniform1f(loc, value);

        }

       

        // Set color uniforms
        for (auto& it : mat->vColor)
        {
            std::string name = it.first;
            Color3 value = it.second;

            std::string nm = name + ".c";
            int loc = glGetUniformLocation(shader->programId, nm.c_str());
            glUniform3fv(loc, 1, &value[0]);
        }

        // Set vec3 uniforms
        for (auto& it : mat->vVec3)
        {
            std::string name = it.first;
            glm::vec3 value = it.second.val;

            int loc = glGetUniformLocation(shader->programId, name.c_str());
            glUniform3fv(loc, 1, &value[0]);
        }

        // Set vec2 uniforms
        for (auto& it : mat->vVec2)
        {
            std::string name = it.first;
            glm::vec2 value = it.second.val;

            int loc = glGetUniformLocation(shader->programId, name.c_str());
            glUniform2fv(loc, 1, &value[0]);
        }

       


    }
}

