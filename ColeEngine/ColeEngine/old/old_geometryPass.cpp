
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

			
			// For every material used in scene
			for (auto& it : engine.Resource().materials)
			{
				std::string matName = it.first;
				Material* mat = it.second;

				// Material exists
				if (mat)
				{
					unsigned int nRefs = mat->entityRefs.size();

					// Use shader on material
					mat->getShader()->UseShader();

					// Render every entity using this material
					for (unsigned int i = 0; i < nRefs; i++)
					{
						Entity* e = mat->entityRefs[i];

						// Entity exists
						if (e)
						{
							// Get it's transform and render components
							TransformComponent* transform = e->getComponent<TransformComponent>();
							RenderComponent* render = e->getComponent<RenderComponent>();

							// Skip if they don't exist
							if (!transform || !render)
								continue;

							// Make object model matrix
							glm::mat4 modelMatrix(1.0f);
							modelMatrix = glm::translate(modelMatrix, transform->pos);
							modelMatrix = glm::rotate(modelMatrix, (transform->angle), transform->rot);
							modelMatrix = glm::scale(modelMatrix, transform->scl * glm::vec3(0.01f, 0.01f, 0.01f));

							// Set model matrix uniform
							int loc = glGetUniformLocation(mat->getShader()->programId, "ModelTr");
							glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

							

							// Set world specific uniforms
							geometryPass->setGeometryPassUnis(engine, geoPassUnis);

							// Set material specific uniforms
							geometryPass->setMaterialUniforms(engine, mat);
							
							// Render mesh
							geometryPass->draw(render);
						}
					}

					// Done using this materials shader
					mat->getShader()->UnuseShader();
				}
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
	// Bind default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

