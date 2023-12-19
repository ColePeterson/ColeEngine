#include "RenderSystem.h"

#include "Component.h"
#include "Entity.h"
#include "Transform.h"
#include "Logging.h"



static unsigned int VaoFromPoints(std::vector<glm::vec4> Pnt, std::vector<int> Ind)
{
	unsigned int vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	GLuint Pbuff;
	glGenBuffers(1, &Pbuff);
	glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(),
		&Pnt[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLuint Ibuff;
	glGenBuffers(1, &Ibuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * Ind.size(),
		&Ind[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
	return vaoID;
}


RenderSystem::RenderSystem(Engine& _engine)
	: engine(_engine), shadowPass(nullptr), lightingPass(nullptr), pointLightPass(nullptr)
{
	// Create pipeline for shadow pass
	shadowPass = new RenderPipeline("shadow pipeline", engine.Resource().shader("shadows_default"));

	// Create pipeline for main image color
	lightingPass = new RenderPipeline("lighting pipeline", engine.Resource().shader("lighting_default"));

	// Create pipeline for point light shadows
	pointLightPass = new RenderPipeline("point light shadow pipeline", engine.Resource().shader("point_shadows_default"));

	engine.Resource().loadShader("debug", "debug.frag", "debug.vert");
	debugShader = engine.Resource().shader("debug");

	// Create frame buffer for shadow map
	shadowFBO.CreateFBO_2D(4096, 4096, false);

	// Create frame buffer for point light shadow map
	pointShadowFBO.CreateFBO_3D(1024, 1024, false);

	// Create VAO for line segment used for debug drawing
	std::vector<glm::vec4> Pnt = { glm::vec4(0,0,0,1), glm::vec4(1,1,1,1) };
	std::vector<int> Ind = { 0,1 };
	gridVAO = VaoFromPoints(Pnt, Ind);

	debugAABB.createVAO();
	debugGimbal.createVAO();
	debugSphere.createVAO(64);
	debugTriangles.createVAO();

	
}






//---------------------------------------------------------------\\
//                       SHADOW PASS                              \\
//----------------------------------------------------------------\\

void RenderSystem::doDirectionalShadowPass(Engine& engine)
{
	glEnable(GL_DEPTH_TEST);

	if (shadowPass && !engine.debug.disableShadowPass)
	{
		shadowPass->shader->UseShader();

		// Drawing to the shadow FBO
		shadowFBO.BindFBO();

		// Clear the screen
		glViewport(0, 0, shadowFBO.width, shadowFBO.height);
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Build shadow transforms
		glm::vec3 lightDir = glm::normalize(-engine.getWorld().lightPos); // Light is always pointing toward center of scene
		shadowPassUnis.lightView = LookAt(engine.getWorld().lightPos, lightDir, glm::vec3(0.0f, 0.0f, 1.0f));
		shadowPassUnis.lightProj = Perspective((engine.getWorld().ry * shadowFBO.width) / shadowFBO.height, engine.getWorld().ry, engine.getWorld().front, engine.getWorld().back);

		// Shadow acne fix
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// Draw all objects
		for (Entity* e : engine.getWorld().entities)
		{
			if (e)
			{
				RenderComponent* render = e->getComponent<RenderComponent>();
				TransformComponent* transform = e->getComponent<TransformComponent>();

				if (render)
				{
					// Make object transform
					glm::mat4 modelMatrix(1.0f);
					modelMatrix = glm::translate(modelMatrix, transform->pos);
					modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
					modelMatrix = glm::scale(modelMatrix, transform->scl);
					shadowPassUnis.transform = modelMatrix;

					// Set shadow pass uniforms
					shadowPass->setShadowPassUnis(engine, render, shadowPassUnis);

					// Render mesh
					shadowPass->draw(render);
				}
			}
		}

		glDisable(GL_CULL_FACE);

		shadowPass->shader->UnuseShader();
		shadowFBO.UnbindFBO();
		glBindTexture(GL_TEXTURE_2D, 0); // Bind default FBO


		

	}
}






//---------------------------------------------------------------\\
//                       POINT LIGHT SHADOW PASS                  \\
//----------------------------------------------------------------\\

void RenderSystem::doPointLightShadowPass(Engine& engine)
{
	//////// Point light shadows //////////////
	glEnable(GL_DEPTH_TEST);

	// Use point light shader program
	pointLightPass->shader->UseShader();
	
	


	float aspect = 1024.0f / 1024.0f; // TODO: no hardcode

	pointLightUnis.farPlane = 60.0f;
	pointLightUnis.pointLightPos = engine.getWorld().lightPos;

	pointLightUnis.shadowProj = glm::perspective(glm::radians(90.0f), aspect, 1.0f, pointLightUnis.farPlane);
	pointLightUnis.textureID = pointShadowFBO.textureCube->get();

	pointLightUnis.shadowTransforms.clear();

	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	pointLightUnis.shadowTransforms.push_back(pointLightUnis.shadowProj *
		glm::lookAt(pointLightUnis.pointLightPos, pointLightUnis.pointLightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


	// Setup viewport for rendering to cubemap faces
	glViewport(0, 0, 1024, 1024);
	pointShadowFBO.BindFBO();

	// Clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	
	// Set per pass/per light shader uniforms
	pointLightPass->setPointShadowPassUnis(engine, pointLightUnis);
	

	// Draw all objects
	for (Entity* e : engine.getWorld().entities)
	{
		if (e)
		{
			RenderComponent* render = e->getComponent<RenderComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();

			if (render)
			{
				// Make object transform
				glm::mat4 modelMatrix(1.0f);
				modelMatrix = glm::translate(modelMatrix, transform->pos);
				modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
				modelMatrix = glm::scale(modelMatrix, transform->scl);

				// Set object transform uniform
				int loc = glGetUniformLocation(pointLightPass->shader->programId, "ModelTr");
				glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));
				
				// Render mesh
				pointLightPass->draw(render);
			}
		}
	}

	pointLightPass->shader->UnuseShader();
	pointShadowFBO.UnbindFBO();
}

//---------------------------------------------------------------\\
//                    LIGHTING PASS                              \\
//----------------------------------------------------------------\\

void RenderSystem::doLightingPass(Engine& engine)
{
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, engine.getPlatform().width, engine.getPlatform().height);

	// Clear the screen
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Set world uniforms
	lightingPassUnis.worldView = engine.getWorld().worldView;
	lightingPassUnis.worldProj = engine.getWorld().worldProj;
	lightingPassUnis.worldInverse = engine.getWorld().worldInverse;
	lightingPassUnis.sunPos = engine.getWorld().lightPos;
	lightingPassUnis.ambientColor = engine.getWorld().ambientColor;
	lightingPassUnis.ambientStrength = engine.getWorld().ambientStrength;

	glPolygonOffset(1.0, 1.0);
	glEnable(GL_POLYGON_OFFSET_FILL);


	int count = 0;

	// For every material used in scene
	for (auto& it : engine.Resource().materials)
	{
		std::string matName = it.first;
		Material* mat = it.second;

		if (mat)
		{
			unsigned int nRefs = mat->entityRefs.size();

			mat->getShader()->UseShader();

			

			// Render every entity using this material
			for (unsigned int i = 0; i < nRefs; i++)
			{
				Entity* e = mat->entityRefs[i];

				if (e)
				{
					TransformComponent* transform = e->getComponent<TransformComponent>();
					RenderComponent* render = e->getComponent<RenderComponent>();

					if (!transform || !render)
						continue;


					if (shadowFBO.texture2D)
					{
						lightingPassUnis.shadowMapID = shadowFBO.texture2D->get();
					}
					else
					{
						Log::warning("Can't set shadowmap ID uniform in lighting pass. Shadow FBO texture doesn't exist");
					}


					// Make object transform
					glm::mat4 modelMatrix(1.0f);
					modelMatrix = glm::translate(modelMatrix, transform->pos);
					modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
					modelMatrix = glm::scale(modelMatrix, transform->scl);
					lightingPassUnis.transform = modelMatrix;

					lightingPassUnis.shadowMatrix = glm::translate(glm::vec3(0.5, 0.5, 0.5)) * glm::scale(glm::vec3(0.5, 0.5, 0.5)) * shadowPassUnis.lightProj * shadowPassUnis.lightView;


					// Render base color pass
					if (lightingPass)
					{
						// Set uniforms for this material
						lightingPass->setMaterialUniforms(engine, lightingPassUnis, pointLightUnis, mat);

						// Point light shadow map texture
						glActiveTexture(GL_TEXTURE4);
						glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightUnis.textureID);
						int loc = glGetUniformLocation(mat->getShader()->programId, "shadowMapPoint");
						glUniform1i(loc, 4);

						// Render mesh
						lightingPass->draw(render);
						count++;
					}
				}
			}

			mat->getShader()->UnuseShader();
		}


	}

	glDisable(GL_POLYGON_OFFSET_FILL);
}





//-------------\\
// DEBUG PASS \\
//-------------\\

void RenderSystem::doDebugPass(Engine& engine)
{
	//debugSphere.draw(engine.getWorld().entities[3],engine.getWorld().eye, lightingPass->shader);
	//debugTriangles.draw(engine.getWorld().entities[3], lightingPass->shader);

	if (debugShader)
	{
		debugShader->UseShader();

		int loc = glGetUniformLocation(debugShader->programId, "WorldProj");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldProj));

		loc = glGetUniformLocation(debugShader->programId, "WorldView");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldView));

		loc = glGetUniformLocation(debugShader->programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldInverse));


		if (engine.selectedEntity)
		{
			TransformComponent* transform = engine.selectedEntity->getComponent<TransformComponent>();
			RenderComponent* render = engine.selectedEntity->getComponent<RenderComponent>();

			glm::mat4 ModelTr(1.0f);
			ModelTr = glm::translate(ModelTr, transform->pos);
			ModelTr = glm::rotate(ModelTr, transform->angle, transform->rot);
			ModelTr = glm::scale(ModelTr, transform->scl);

			loc = glGetUniformLocation(debugShader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ModelTr));

			if (engine.onSelect)
			{
				debugAABB.needsUpdate = true;
				debugSphere.needsUpdate = true;
			}


			//debugAABB.draw(engine.selectedEntity, debugShader);

			glDisable(GL_DEPTH_TEST);
			debugGimbal.draw(engine.selectedEntity, debugShader);
			//debugTriangles.draw(engine.selectedEntity, debugShader);
			//debugSphere.draw(engine.selectedEntity, engine.getWorld().eye, debugShader);
		}

		debugGimbal.draw(engine.getWorld().lightPos, debugShader);

		//drawBVHTreeDebug();

	}

	// After drawing all scene objects, draw needed debug lines
	if (engine.debug.drawDebugLines)
	{
		//drawGridLines(engine);
	}

	debugShader->UnuseShader();
}

void RenderSystem::update(Engine& engine)
{

	// Render scene from light perspective
	//doDirectionalShadowPass(engine);

	// Render scene from point light perspective
	doPointLightShadowPass(engine);

	// Render scene from camera perspective with base lighting
	doLightingPass(engine);
	
	// Draw debug lines
	doDebugPass(engine); 

	
}



void RenderSystem::drawTreeRec(TreeNode* node, int level, int maxLevel)
{
	if (level >= maxLevel)
		return;

	if (node)
	{

		glm::vec3 color = { 0.0f, 0.0f, 0.0f };
		if (level == 0)
			color = { 1.0f, 0.0f, 0.0f };
		else if (level == 1)
			color = { 0.0f, 1.0f, 0.0f };
		else if (level == 2)
			color = { 0.0f, 0.0f, 1.0f };
		else if (level == 3)
			color = { 1.0f, 1.0f, 1.0f };
		else if (level == 4)
			color = { 0.0f, 0.0f, 1.0f };
		else if (level == 5)
			color = { 1.0f, 0.0f, 1.0f };
		else if (level == 6)
			color = { 0.0f, 1.0f, 1.0f };



		Box3D* box = node->box;

		glm::vec3 minP = box->center - box->extents;
		glm::vec3 maxP = box->center + box->extents;

		glm::mat4 transform = Translate(minP) * Scale(maxP - minP);

		// Set lines color for fragment shader
		int loc = glGetUniformLocation(debugShader->programId, "diffuse");
		glUniform3fv(loc, 1, &color[0]);

		// Set transformation for vertex shader
		loc = glGetUniformLocation(debugShader->programId, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));


		// Draw
		glBindVertexArray(debugAABB.vaoID);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Recurse
		drawTreeRec(node->right, ++level, maxLevel);
		drawTreeRec(node->left, level, maxLevel);

	}
}

void RenderSystem::drawBVHTreeDebug()
{
	drawTreeRec(engine.getWorld().bvh, 0, (int)engine.debug.float1);
}


void RenderSystem::drawGridLines(Engine& engine)
{

	if (lightingPass)
	{
		lightingPass->shader->UseShader();

		glm::vec3 col = { 0.0f, 0.0f, 0.0f };
		
		int loc = glGetUniformLocation(lightingPass->shader->programId, "objectId");
		glUniform1i(loc, 5);

		loc = glGetUniformLocation(lightingPass->shader->programId, "diffuse");
		glUniform3fv(loc, 1, &col[0]);


		for (int i = 0; i < 40; i++)
		{
			float inc = static_cast<float>(i) * 2.0f;

			glm::vec3 A = { -GRIDLINE_LENGTH, inc, 0.0f };
			glm::vec3 B = { GRIDLINE_LENGTH, inc, 0.0f };

			glm::mat4 trn = Translate(A) * Scale(B - A);

			loc = glGetUniformLocation(lightingPass->shader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

			glBindVertexArray(gridVAO);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			A = { -GRIDLINE_LENGTH, -inc, 0.0f };
			B = { GRIDLINE_LENGTH, -inc, 0.0f };

			trn = Translate(A) * Scale(B - A);

			loc = glGetUniformLocation(lightingPass->shader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

			glBindVertexArray(gridVAO);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		for (int i = 0; i < 20; i++)
		{
			float inc = static_cast<float>(i) * 2.0f;

			glm::vec3 A = {inc , -GRIDLINE_LENGTH, 0.0f };
			glm::vec3 B = {inc , GRIDLINE_LENGTH, 0.0f };

			glm::mat4 trn = Translate(A) * Scale(B - A);

			loc = glGetUniformLocation(lightingPass->shader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

			glBindVertexArray(gridVAO);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			A = { -inc , -GRIDLINE_LENGTH, 0.0f };
			B = { -inc , GRIDLINE_LENGTH, 0.0f };

			trn = Translate(A) * Scale(B - A);

			loc = glGetUniformLocation(lightingPass->shader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

			glBindVertexArray(gridVAO);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		col = { 0.0f, 1.0f, 0.0f };

		glm::vec3 A = { 0.0f , 0.0f, -GRIDLINE_LENGTH };
		glm::vec3 B = { 0.0f , 0.0f, GRIDLINE_LENGTH };

		glm::mat4 trn = Translate(A) * Scale(B - A);

		loc = glGetUniformLocation(lightingPass->shader->programId, "diffuse");
		glUniform3fv(loc, 1, &col[0]);

		loc = glGetUniformLocation(lightingPass->shader->programId, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

		glBindVertexArray(gridVAO);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		
		lightingPass->shader->UnuseShader();
	}
}
