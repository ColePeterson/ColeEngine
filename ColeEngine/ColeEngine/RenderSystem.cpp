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

static glm::vec3 getRayDirection(int screenWidth, int screenHeight, double mouseX, double mouseY, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	// Convert mouse coordinates to NDC (Normalized Device Coordinates)
	float ndcX = (2.0f * mouseX) / (static_cast<float>(screenWidth) - 1.0f);
	float ndcY = 1.0f - (2.0f * mouseY) / static_cast<float>(screenHeight);
	//float ndcX = 0.0f;
	//float ndcY = 0.0f;
	// Create a ray in clip space
	glm::vec4 rayClip(ndcX, ndcY, -1.0f, 0.0f);

	glm::mat4 viewProjectionInverse = glm::inverse(projectionMatrix * viewMatrix);

	// Unproject the ray into view space
	glm::vec4 rayView = viewProjectionInverse * rayClip;
	rayView /= rayView.w;

	// The ray direction is the vector from the camera position to the unprojected point
	glm::vec3 rayDirection = glm::normalize(glm::vec3(rayView) - glm::vec3(viewMatrix[3]));

	return rayDirection;
}

RenderSystem::RenderSystem(Engine& _engine)
	: engine(_engine), lightingPass(nullptr), geometryPass(nullptr), 
	pointLightPass(nullptr), gBuffer(nullptr)
{
	// Create pipeline for rendering geomentry into G-buffer
	geometryPass = new RenderPipeline("Geometry pipeline", engine.Resource().shader("geometry_default"));

	// Create pipeline for deffered lighting pass
	lightingPass = new RenderPipeline("Lighting pipeline", engine.Resource().shader("lighting"));

	// Create pipeline for point light shadows
	pointLightPass = new RenderPipeline("Point light shadow pipeline", engine.Resource().shader("point_shadows_default"));

	// Create pipeline for post processing
	postProcessPass = new RenderPipeline("Post process pipeline", engine.Resource().shader("post_process_default"));


	particlesPass = new RenderPipeline("Particles pipeline", engine.Resource().shader("particles_default"));

	// Load debug shader
	engine.Resource().loadShader("debug", "debug.frag", "debug.vert");
	debugShader = engine.Resource().shader("debug");


	// Create pipeline for shadow pass
	//shadowPass = new RenderPipeline("shadow pipeline", engine.Resource().shader("shadows_default"));

	// Create frame buffer for shadow map
	//shadowFBO.CreateFBO_2D(1024, 1024, false);


	// Create G-Buffer for deffered shadings
	gBuffer = new GBuffer(engine.getPlatform().width, engine.getPlatform().height);

	// Create the texture attachments for G-Buffer
	gBuffer->addBuffer(BufferType::POSITION);
	gBuffer->addBuffer(BufferType::NORMALS);
	gBuffer->addBuffer(BufferType::ALBEDO);
	gBuffer->addBuffer(BufferType::SPECULAR);
	//gBuffer->addBuffer(BufferType::VIEW);

	// Create framebuffer for rendering scene color
	sceneColorFBO = new FrameBuffer(engine.getPlatform().width, engine.getPlatform().height);
	//sceneColorFBO->addTexture("scene color");

	// Create VAO for line segment used for debug drawing
	std::vector<glm::vec4> Pnt = { glm::vec4(0,0,0,1), glm::vec4(1,1,1,1) };
	std::vector<int> Ind = { 0,1 };
	gridVAO = VaoFromPoints(Pnt, Ind);

	// Create debug shape VAO's
	debugAABB.createVAO();
	debugGimbal.createVAO();
	debugSphere.createVAO(64);
	debugTriangles.createVAO();

}

//---------------------------------------------------------------\\
//                    GEOMETRY PASS                              \\
//----------------------------------------------------------------\\

void RenderSystem::drawSpriteParticles()
{
	// Set viewport to G-Buffer size (Screen size)
	//glViewport(0, 0, gBuffer->getWidth(), gBuffer->getHeight());

	// Drawing to the G-Buffer frame buffer
	//glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->getBuffer());


	// For every particle system in world
	for (ParticleEmitter* system : engine.getWorld().particles)
	{
		system->update(engine);

		RenderComponent* render = system->getComponent<RenderComponent>();
		TransformComponent* transform = system->getComponent<TransformComponent>();

		// For every particle in system
		for (Particle& p : system->particles)
		{
			// The render component's mesh
			Mesh* mesh = render->mesh;

			// Mesh exists and particle is alive
			if (mesh && p.life > 0.0f)
			{
				unsigned int nSubMeshes = mesh->nMeshes;

				// For every sub-mesh in the mesh
				for (unsigned int i = 0; i < nSubMeshes; i++)
				{
					unsigned int materialIndex = mesh->meshData[i].materialIndex;

					// If sub-mesh material index is greater than # of material slots, set it to 
					if (materialIndex >= render->materials.size())
					{
						materialIndex = 0;
					}


					// The material on this sub-mesh
					Material* mat = render->materials[materialIndex];

					// Material exists
					if (mat)
					{
						mat->getShader()->UseShader();

						// Make object model matrix
						glm::mat4 modelMatrix(1.0f);
						modelMatrix = glm::translate(modelMatrix, p.pos);
						modelMatrix = glm::scale(modelMatrix, transform->scl * p.scale * glm::vec3(0.01f, 0.01f, 0.01f));

						// Face camera
						glm::vec3 look = normalize(engine.getWorld().eyePos - p.pos);
						glm::vec3 right = cross(glm::vec3(0.0f, 0.0f, 1.0f), look);
						glm::vec3 up2 = cross(look, right);
						glm::mat4 lookTransform;
						lookTransform[0] = glm::vec4(right, 0);
						lookTransform[1] = glm::vec4(up2, 0);
						lookTransform[2] = glm::vec4(look, 0);

						modelMatrix *= lookTransform;


						// Set model matrix uniform
						int loc = glGetUniformLocation(mat->getShader()->programId, "ModelTr");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

						//loc = glGetUniformLocation(mat->getShader()->programId, "hasDiffuseTexture");
						//glUniform1i(loc, mat->hasDiffuseTexture);

						//loc = glGetUniformLocation(mat->getShader()->programId, "hasNormalsTexture");
						//glUniform1i(loc, mat->hasNormalsTexture);

						// Set per sprite uniforms
						SpriteUniforms spriteUnis;
						spriteUnis.color = p.col;

						// World specific uniforms
						loc = glGetUniformLocation(mat->getShader()->programId, "WorldProj");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldProj));

						loc = glGetUniformLocation(mat->getShader()->programId, "WorldView");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldView));

						loc = glGetUniformLocation(mat->getShader()->programId, "WorldInverse");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldInverse));

						loc = glGetUniformLocation(mat->getShader()->programId, "time");
						glUniform1f(loc, engine.getWorld().time);

						geometryPass->setSpriteMaterialUniforms(engine, mat, spriteUnis);

						

						// Bind the VAO
						glBindVertexArray(mesh->meshData[i].VAO);

						// Draw the mesh
						glDrawElements(GL_TRIANGLES, mesh->meshData[i].indices.size(), GL_UNSIGNED_INT, 0);

						// Un-bind the VAO
						glBindVertexArray(0);

						// Done using this materials shader
						mat->getShader()->UnuseShader();
					}

				}

			}
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void RenderSystem::drawMeshParticlesShadow(ShaderProgram* shader)
{
	
	// For every particle system in world
	for (ParticleEmitter* system : engine.getWorld().particles)
	{
		system->update(engine);

		RenderComponent* render = system->getComponent<RenderComponent>();
		TransformComponent* transform = system->getComponent<TransformComponent>();

		// For every particle in system
		for (Particle& p : system->particles)
		{
			// The render component's mesh
			Mesh* mesh = render->mesh;

			// Mesh exists and particle is alive
			if (mesh && p.life > 0.0f)
			{
				unsigned int nSubMeshes = mesh->nMeshes;

				// For every sub-mesh in the mesh
				for (unsigned int i = 0; i < nSubMeshes; i++)
				{
					unsigned int materialIndex = mesh->meshData[i].materialIndex;

					// If sub-mesh material index is greater than # of material slots, set it to 
					if (materialIndex >= render->materials.size())
					{
						materialIndex = 0;
					}

					// Make object model matrix
					glm::mat4 modelMatrix(1.0f);
					modelMatrix = glm::translate(modelMatrix, p.pos);
					modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
					modelMatrix = glm::scale(modelMatrix, transform->scl * p.scale * glm::vec3(0.01f, 0.01f, 0.01f));

					// Set model matrix uniform
					int loc = glGetUniformLocation(shader->programId, "ModelTr");
					glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

					
					// Set world specific uniforms
					//geometryPass->setGeometryPassUnis(engine, geoPassUnis);

					// Set material specific uniforms
					//geometryPass->setMaterialUniforms(engine, mat);


					loc = glGetUniformLocation(shader->programId, "diffuse.c");
					glUniform4fv(loc, 1, &p.col[0]);

					// Bind the VAO
					glBindVertexArray(mesh->meshData[i].VAO);

					// Draw the mesh
					glDrawElements(GL_TRIANGLES, mesh->meshData[i].indices.size(), GL_UNSIGNED_INT, 0);

					// Un-bind the VAO
					glBindVertexArray(0);

					
					

				}

			}
		}
	}
}

void RenderSystem::drawMeshParticles()
{
	// Set viewport to G-Buffer size (Screen size)
	glViewport(0, 0, gBuffer->getWidth(), gBuffer->getHeight());

	// Drawing to the G-Buffer frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->getBuffer());

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//glEnable(GL_BLEND);

	// For every particle system in world
	for (ParticleEmitter* system : engine.getWorld().particles)
	{
		system->update(engine);

		RenderComponent* render = system->getComponent<RenderComponent>();
		TransformComponent* transform = system->getComponent<TransformComponent>();

		// For every particle in system
		for (Particle& p : system->particles)
		{
			// The render component's mesh
			Mesh* mesh = render->mesh;

			// Mesh exists and particle is alive
			if (mesh && p.life > 0.0f)
			{
				unsigned int nSubMeshes = mesh->nMeshes;

				// For every sub-mesh in the mesh
				for (unsigned int i = 0; i < nSubMeshes; i++)
				{
					unsigned int materialIndex = mesh->meshData[i].materialIndex;

					// If sub-mesh material index is greater than # of material slots, set it to 
					if (materialIndex >= render->materials.size())
					{
						materialIndex = 0;
					}


					// The material on this sub-mesh
					Material* mat = render->materials[materialIndex];

					// Material exists
					if (mat)
					{
						mat->getShader()->UseShader();

						// Make object model matrix
						glm::mat4 modelMatrix(1.0f);
						modelMatrix = glm::translate(modelMatrix, p.pos);
						modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
						modelMatrix = glm::scale(modelMatrix, transform->scl * p.scale * glm::vec3(0.01f, 0.01f, 0.01f));

						// Set model matrix uniform
						int loc = glGetUniformLocation(mat->getShader()->programId, "ModelTr");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

						loc = glGetUniformLocation(mat->getShader()->programId, "hasDiffuseTexture");
						glUniform1i(loc, mat->hasDiffuseTexture);

						loc = glGetUniformLocation(mat->getShader()->programId, "hasNormalsTexture");
						glUniform1i(loc, mat->hasNormalsTexture);

						// Set world specific uniforms
						geometryPass->setGeometryPassUnis(engine, geoPassUnis);

						// Set material specific uniforms
						geometryPass->setMaterialUniforms(engine, mat);


						


						loc = glGetUniformLocation(mat->getShader()->programId, "diffuse.c");
						glUniform4fv(loc, 1, &p.col[0]);

						// Bind the VAO
						glBindVertexArray(mesh->meshData[i].VAO);

						// Draw the mesh
						glDrawElements(GL_TRIANGLES, mesh->meshData[i].indices.size(), GL_UNSIGNED_INT, 0);

						// Un-bind the VAO
						glBindVertexArray(0);

						// Done using this materials shader
						mat->getShader()->UnuseShader();
					}

				}

			}
		}
	}
}


void RenderSystem::doGeometryPass(Engine& engine)
{
	glEnable(GL_DEPTH_TEST);

	if (geometryPass)
	{
		if (geometryPass->shader && gBuffer)
		{
			// Set viewport to G-Buffer size (Screen size)
			glViewport(0, 0, gBuffer->getWidth(), gBuffer->getHeight());

			// Drawing to the G-Buffer frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->getBuffer());

			// Clear buffer
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPolygonOffset(1.0, 1.0);
			glEnable(GL_POLYGON_OFFSET_FILL);

			// World specific uniforms
			geoPassUnis.worldView = engine.getWorld().worldView;
			geoPassUnis.worldProj = engine.getWorld().worldProj;
			geoPassUnis.worldInverse = engine.getWorld().worldInverse;

			
			// For every entity in world
			for (Entity* e : engine.getWorld().entities)
			{
				if (e)
				{
					RenderComponent* render = e->getComponent<RenderComponent>();
					TransformComponent* transform = e->getComponent<TransformComponent>();

					// Skip if they don't exist
					if (!transform || !render)
						continue;

					// The render component's mesh
					Mesh* mesh = render->mesh;

					// Mesh exists
					if (mesh)
					{
						unsigned int nSubMeshes = mesh->nMeshes;

						// For every sub-mesh in the mesh
						for (unsigned int i = 0; i < nSubMeshes; i++)
						{
							unsigned int materialIndex = mesh->meshData[i].materialIndex;

							// If sub-mesh material index is greater than # of material slots, set it to 
							if (materialIndex >= render->materials.size())
							{
								materialIndex = 0;
							}

							
							// The material on this sub-mesh
							Material* mat = render->materials[materialIndex];

							// Material exists
							if (mat)
							{
								mat->getShader()->UseShader();

								// Make object model matrix
								glm::mat4 modelMatrix(1.0f);
								modelMatrix = glm::translate(modelMatrix, transform->pos);
								modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
								modelMatrix = glm::scale(modelMatrix, transform->scl * glm::vec3(0.01f, 0.01f, 0.01f));

								// Set model matrix uniform
								int loc = glGetUniformLocation(mat->getShader()->programId, "ModelTr");
								glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

								loc = glGetUniformLocation(mat->getShader()->programId, "hasDiffuseTexture");
								glUniform1i(loc, mat->hasDiffuseTexture);

								loc = glGetUniformLocation(mat->getShader()->programId, "hasNormalsTexture");
								glUniform1i(loc, mat->hasNormalsTexture);

								// Set world specific uniforms
								geometryPass->setGeometryPassUnis(engine, geoPassUnis);

								// Set material specific uniforms
								geometryPass->setMaterialUniforms(engine, mat);


								// Bind the VAO
								glBindVertexArray(mesh->meshData[i].VAO);

								// Draw the mesh
								glDrawElements(GL_TRIANGLES, mesh->meshData[i].indices.size(), GL_UNSIGNED_INT, 0);

								// Un-bind the VAO
								glBindVertexArray(0);

								// Done using this materials shader
								mat->getShader()->UnuseShader();
							}
							else
							{
								Log::warning("Material on sub-mesh dpesn't exist!");
							}
							
						}
					}
				}
			}
			glDisable(GL_POLYGON_OFFSET_FILL);

			// ------ Render Particles ------ \\

			// Set viewport to G-Buffer size (Screen size)
			glViewport(0, 0, gBuffer->getWidth(), gBuffer->getHeight());

			// Drawing to the G-Buffer frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->getBuffer());
			
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			
			//glDepthFunc(GL_ALWAYS);
			drawMeshParticles();

			glDisable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glDepthFunc(GL_LESS);
			

			// Bind default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			

		}

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDisable(GL_BLEND);
		

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
	

	// Render to cubemap for every point light
	unsigned int nLights = engine.getWorld().pointLights.size();
	for (int i = 0; i < nLights; i++)
	{
		PointLight* light = engine.getWorld().pointLights[i];
		TransformComponent* trn = light->getComponent<TransformComponent>();
		PointLightComponent* plc = light->getComponent<PointLightComponent>();


		// Setup viewport for rendering to cubemap faces
		glViewport(0, 0, light->getShadowFBO()->width, light->getShadowFBO()->height);

		// Bind shadowmap FBO
		light->getShadowFBO()->BindFBO();

		// Clear depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);

		float aspect = light->getShadowFBO()->width / light->getShadowFBO()->height;

		pointLightUnis.farPlane = plc->farPlane;
		pointLightUnis.pointLightPos = trn->pos;

		pointLightUnis.shadowProj = glm::perspective(90.0f, aspect, 1.0f, pointLightUnis.farPlane);
		pointLightUnis.textureID = light->getShadowFBO()->textureCube->get();
		
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
					modelMatrix = glm::scale(modelMatrix, transform->scl * glm::vec3(0.01f, 0.01f, 0.01f));

					// Set object transform uniform
					int loc = glGetUniformLocation(pointLightPass->shader->programId, "ModelTr");
					glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

					// Render mesh
					pointLightPass->draw(render);
				}
			}
		}

		

		drawMeshParticlesShadow(pointLightPass->shader);

		light->getShadowFBO()->UnbindFBO();
	}

	pointLightPass->shader->UnuseShader();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


//---------------------------------------------------------------\\
//                    LIGHTING PASS                              \\
//----------------------------------------------------------------\\

void RenderSystem::doLightingPass(Engine& engine)
{
	if (lightingPass)
	{
		if (lightingPass->shader && gBuffer)
		{

			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, engine.getPlatform().width, engine.getPlatform().height);

			// Bind default frame buffer
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, sceneColorFBO->get());

			// Clear screen
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			lightingPass->shader->UseShader();

			// Set lighting uniforms
			lightingPass->setLightingPassUnis(engine, gBuffer, lightingPass2Unis);


			// Draw full screen quad
			if (gBuffer->getQuad())
			{
				glBindVertexArray(gBuffer->getQuad()->vaoID);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindVertexArray(0);
			}
			else
			{
				Log::error("Cannot draw G-Buffer Quad. It does not exist.");
			}


			//glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->getBuffer());
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
			// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
			// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
			// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
			//glBlitFramebuffer(0, 0, gBuffer->getWidth(), gBuffer->getHeight(), 0, 0, gBuffer->getWidth(), gBuffer->getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);


		}
	}
}


void RenderSystem::doPostProcessPass(Engine& engine)
{
	if (postProcessPass)
	{
		if (postProcessPass->shader && sceneColorFBO)
		{
			// Bind default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, engine.getPlatform().width, engine.getPlatform().height);

			// Clear screen
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			postProcessPass->shader->UseShader();

			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, sceneColorFBO->getTextures()[0].texture->get());
			int loc = glGetUniformLocation(postProcessPass->shader->programId, "sceneColor");
			glUniform1i(loc, 0);

			loc = glGetUniformLocation(postProcessPass->shader->programId, "time");
			glUniform1f(loc, engine.getWorld().time);


			// Draw full screen quad
			if (sceneColorFBO->getQuad())
			{
				glBindVertexArray(sceneColorFBO->getQuad()->vaoID);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindVertexArray(0);
			}
			else
			{
				Log::error("Cannot draw post process Quad. It does not exist.");
			}

			postProcessPass->shader->UnuseShader();
		}

		
	}
}







//-------------\\
// DEBUG PASS \\
//-------------\\

void RenderSystem::doDebugPass(Engine& engine)
{
	

	if (debugShader)
	{
		glDisable(GL_DEPTH_TEST);

		debugShader->UseShader();

		int loc = glGetUniformLocation(debugShader->programId, "WorldProj");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldProj));

		loc = glGetUniformLocation(debugShader->programId, "WorldView");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldView));

		loc = glGetUniformLocation(debugShader->programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(engine.getWorld().worldInverse));

		

		// Draw stuff on delected entity
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

			//glEnable(GL_DEPTH_TEST);
			//debugAABB.draw(engine.selectedEntity, debugShader);

			glDisable(GL_DEPTH_TEST);
			debugGimbal.draw(engine.selectedEntity, debugShader);

			/*
			glm::vec3 rayDir = getRayDirection(engine.getPlatform().width, engine.getPlatform().height, 
				engine.getPlatform().mouseX,engine.getPlatform().mouseY, engine.getWorld().worldView, 
				engine.getWorld().worldProj);

			glm::vec3 from = engine.getWorld().eyePos + glm::vec3(1.0f, 0.0f, 0.0f);
			//glm::vec3 to = from - rayDir * 90.0f;
			glm::vec3 to = { 1.0f, 1.0f, 1.0f };
			//glm::vec3 from = { 0.0f, 0.0f, 0.0f };
			//glm::vec3 to = { 0.0f, 0.0f, 20.0f };

			//debugGimbal.drawLine(from, to, debugShader);

			*/

			//debugTriangles.draw(engine.selectedEntity, debugShader);
			//debugSphere.draw(engine.selectedEntity, engine.getWorld().eye, debugShader);
		}
		
		//debugGimbal.draw(engine.getWorld().lightPos, debugShader);

		//drawTriangleAABB();
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

	// Render geometry into G_Buffer
	doGeometryPass(engine);

	// Render deferred lighting
	doLightingPass(engine);


	/*
	if (engine.mode == EngineMode::TERRAIN)
	{
		Platform& platform = engine.getPlatform();

		int u = static_cast<int>(platform.mouseX);
		int v = static_cast<int>(platform.mouseY);

		int width = static_cast<int>(platform.width);
		int height = static_cast<int>(platform.height);

		//GLubyte* pixels = new GLubyte[4 * width * height];

		GLubyte pixel[4];

		// Read positions
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glReadPixels(u, v, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

		glm::vec3 mousePos = { pixel[0], pixel[1], pixel[2] };

		engine.getWorld().mouseWorldPos = mousePos;

		//std::cout << "x: " << mousePos.x << "\ny: " << mousePos.y << "\n:z: " << mousePos.z << "\n\n";

	}
	*/

	// Apply post processing
	doPostProcessPass(engine);

	// Draw debug lines
	doDebugPass(engine); 



	
}

void RenderSystem::drawTriangleAABB()
{
	
	std::vector<Box3D*>& objList = engine.getWorld().objList;

	int i = 0;
	for (Box3D* box : objList)
	{
		if (box)
		{
			glm::vec3 minP = box->center - box->extents;
			glm::vec3 maxP = box->center + box->extents;

			glm::mat4 transform = Translate(minP) * Scale(maxP - minP);

			glm::vec3 color = { 1.0f, 0.0f, 0.0f };

			// Set lines color for fragment shader
			int loc = glGetUniformLocation(debugShader->programId, "diffuse");
			glUniform3fv(loc, 1, &color[0]);

			// Set transformation for vertex shader
			loc = glGetUniformLocation(debugShader->programId, "ModelTr");
			glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

			if (i == (int)engine.debug.float2)
			{
				// Draw
				glBindVertexArray(debugAABB.vaoID);
				glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}
		else
		{
			std::cout << "BOX NO EXIST\n";
		}

		


		i++;
	}
}

void RenderSystem::drawTreeLeaves(TreeNode* node, int level)
{
	if (level >= 800)
		return;

	if (node)
	{
		glm::vec3 color = { 0.0f, 0.0f, 1.0f };

		if (node->left->type == NodeType::LEAF && node->right->type == NodeType::LEAF)
			color = { 1.0f, 0.0f, 1.0f };
		else if(node->left->type == NodeType::LEAF)
			color = { 0.0f, 0.0f, 1.0f };
		else if (node->left->type == NodeType::LEAF)
			color = { 1.0f, 1.0f, 0.0f };

		if (node->left->type == NodeType::LEAF || node->right->type == NodeType::LEAF)
		{

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
			drawTreeLeaves(node->right, ++level);
			drawTreeLeaves(node->left, level);
		}
	}
}


static glm::vec3 randomColor(float x)
{
	return glm::vec3(
		0.5f + 0.5f * cosf(x), 
		0.5f + 0.5f * cosf(x * 2.2f + 0.1f),
		0.5f + 0.5f * cosf(x * 5.5f + 1.5f)
	);
}

void RenderSystem::drawTreeRec(TreeNode* node, int level, int maxLevel)
{
	if (level > maxLevel)
		return;

	if (node )
	{
		glm::vec3 color = { 0.0f, 0.0f, 0.0f };
		
		float x = static_cast<float>(level);

		color = randomColor(x * 4.0f);

		if (node->type == NodeType::LEAF)
		{
			color = { 1.0f, 1.0f, 1.0f };
		}

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
		if (level == maxLevel)
		{
			glBindVertexArray(debugAABB.vaoID);
			glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		// Recurse
		drawTreeRec(node->right, ++level, maxLevel);
		drawTreeRec(node->left, level, maxLevel);

	}
}

void RenderSystem::drawBVHTreeDebug()
{
	drawTreeRec(engine.getWorld().bvh, 0, (int)engine.debug.float1);

	//drawTreeLeaves(engine.getWorld().bvh, 0);
}





RenderSystem::~RenderSystem()
{
	if (geometryPass)
	{
		delete geometryPass;
		geometryPass = nullptr;
	}
	
	if (lightingPass)
	{
		delete lightingPass;
		lightingPass = nullptr;
	}
	if (pointLightPass)
	{
		delete pointLightPass;
		pointLightPass = nullptr;
	}
}


//---------------------------------------------------------------\\
//                       DIRECTIONAL SHADOW PASS (OLD)            \\
//----------------------------------------------------------------\\

/*
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
*/

GBuffer* RenderSystem::getGBuffer()
{
	if (gBuffer)
	{
		return gBuffer;
	}
	else
	{
		Log::error("Can't get G-Buffer because it's null!");
		return nullptr;
	}
}