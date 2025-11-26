#pragma once

#ifndef _RENDERSYSTEM
#define _RENDERSYSTEM


#include "Engine.h"
#include "RenderPipeline.h"
#include "System.h"
#include "FBO.h"
#include "Texture.h"
#include "DebugDrawing.h"
#include "GBuffer.h"
#include "FrameBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/ext.hpp>

#define GRIDLINE_LENGTH 99999.0f




class RenderSystem : public System
{
public:
	explicit RenderSystem(Engine& _engine);
	~RenderSystem();
	void update(Engine& engine) override;
	
private:

	void doGeometryPass(Engine& engine);
	void doLightingPass(Engine& engine);
	void doPointLightShadowPass(Engine& engine);
	void doPostProcessPass(Engine& engine);
	void doDebugPass(Engine& engine);
	
	void drawMeshParticlesShadow(ShaderProgram* shader);
	void drawMeshParticles();
	void drawSpriteParticles();

	void drawTreeRec(TreeNode* node, int level, int maxLevel);
	void drawTreeLeaves(TreeNode* node, int level);
	void drawBVHTreeDebug();
	void drawTriangleAABB();

	GBuffer* getGBuffer();

	// Render pipeline for point light shadows
	RenderPipeline* pointLightPass;

	RenderPipeline* particlesPass;

	// Render pipeline for G-buffer pass
	RenderPipeline* geometryPass;

	// Render pipeline for deferred lighting
	RenderPipeline* lightingPass;

	// Render pipeline for post processing
	RenderPipeline* postProcessPass;

	// Basic shader for debug pass
	ShaderProgram* debugShader;

	// G-Buffer for deferred shading
	GBuffer* gBuffer;

	// Framebuffer to render scene color to
	FrameBuffer* sceneColorFBO;

	//FBO shadowFBO; // FBO for directional shadows
	//glm::mat4 shadowMatrix; // Transform for directional light

	// Stores uniforms
	GeometryPassUniforms geoPassUnis;
	LightingPassUniforms lightingPass2Unis;

	LightingPassUniforms lightingPassUnis;
	ShadowPassUniforms shadowPassUnis;
	PointLightShadowPassUniforms pointLightUnis;

	// Debug objects
	DebugAABB debugAABB;
	DebugGimbal debugGimbal;
	DebugSphere debugSphere;
	DebugTriangles debugTriangles;
	unsigned int gridVAO;

	Engine& engine;
};

#endif