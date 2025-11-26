#pragma once

#ifndef _PARTICLEEMITTER
#define _PARTICLEEMITTER

#include<string>
#include<vector>
#include <functional>
#include <glm/glm.hpp>

#include "Entity.h"
#include "Engine.h"

#include "Component.h"

class Engine;

struct Particle
{
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 vel = { 0.0f, 0.0f, 0.0f };
	glm::vec3 acc = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	glm::vec4 col = { 0.0f, 0.0f, 0.0f, 1.0f };

	float life = 0.0f;
	float lifeInit = 0.0f;
	//float timeAlive = 0.0f;
};



enum class EmitterState
{
	RUNNING,
	STOPPED,
	PAUSED
};

enum class RenderType
{
	MESH,
	BILLBOARD
};





class ParticleEmitter : public Entity
{
public:
	ParticleEmitter(std::string _name, int rate, unsigned int maximum);

	void init();
	void start();
	void stop();

	void update(Engine& engine);

	void setSpawnRate(float rate) { spawnRate = rate; }
	void setSpawnRadius(float radius) { spawnRate = radius; }
	void setMaxParticles(unsigned int mp) { maxParticles = mp; }

	std::vector<Particle> particles;

	

private:

	Particle* curr;

	std::vector<std::function<void()>> initFuncs;
	std::vector<std::function<void()>> updateFuncs;

	TransformComponent* trn;

	RenderType renderType;

	int spawnRate;

	unsigned int maxParticles;


	float systemTime;
	int frame;

	EmitterState state;

	unsigned int lastUsedParticle;

	unsigned int getNextParticle();
	void spawn(Particle& particle);
};

#endif