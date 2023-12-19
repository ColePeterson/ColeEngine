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
	void doDebugPass(Engine& engine);
	//void doDirectionalShadowPass(Engine& engine);

	void drawTreeRec(TreeNode* node, int level, int maxLevel);
	void drawBVHTreeDebug();

	// Render pipeline for shadows
	RenderPipeline* shadowPass;

	// Render pipeline for point light shadows
	RenderPipeline* pointLightPass;

	// Render pipeline for G-buffer pass
	RenderPipeline* geometryPass;

	// Render pipeline for deferred lighting
	RenderPipeline* lightingPass;

	// Basic shader for debug pass
	ShaderProgram* debugShader;

	// G-Buffer for deferred shading
	GBuffer* gBuffer;

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