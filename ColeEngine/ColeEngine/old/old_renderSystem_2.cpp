void RenderSystem::doGeometryPass(Engine& engine)
{
	glEnable(GL_DEPTH_TEST);

	if (geometryPass)
	{
		if (geometryPass->shader && gBuffer)
		{
			// Bind G-Buffer frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->getBuffer());

			// Clear buffer
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use g-buffer pass shader
			geometryPass->shader->UseShader();


			geoPassUnis.worldView = engine.getWorld().worldView;
			geoPassUnis.worldProj = engine.getWorld().worldProj;
			geoPassUnis.worldInverse = engine.getWorld().worldInverse;

			geometryPass->setGeometryPassUnis(engine, geoPassUnis);

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
						int loc = glGetUniformLocation(geometryPass->shader->programId, "ModelTr");
						glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(modelMatrix));

						// Render mesh
						geometryPass->draw(render);
					}
				}
			}

			// un-use g-buffer pass shader
			geometryPass->shader->UnuseShader();

			// Bind default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}
	}
}