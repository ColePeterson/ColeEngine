#include "ParticleEmitter.h"
#include "ParticleFunctions.h"










ParticleEmitter::ParticleEmitter(std::string _name, int rate, unsigned int maximum)
	: spawnRate(rate), maxParticles(maximum), systemTime(0.0f),
	frame(0), lastUsedParticle(0), state(EmitterState::RUNNING), trn(nullptr), renderType(RenderType::BILLBOARD),
	curr(nullptr)
	
{
	setName(_name);

	

}



void ParticleEmitter::init()
{
	TransformComponent* transform = this->getComponent<TransformComponent>();

	
	// Add init functions
	initFuncs.push_back([this]() { setLifeTimeRandom(curr, 1.0f, 4.0f); });
	initFuncs.push_back([this]() { setColorRandom(curr); });
	initFuncs.push_back([this]() { setPositionSphere(curr, 0.4f); });
	initFuncs.push_back([this]() { setVelocityRandom(curr, -3.0f, 3.0f); });

	// Add update functions
	updateFuncs.push_back([this]() { setScaleOverLife(curr, 1.0f, 0.01f); });
	//updateFuncs.push_back([this]() { setColorOverLife(curr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); });
	updateFuncs.push_back([this]() { setAlphaOverLife(curr, 1.0f, 0.0f); });

	if (transform)
	{
		trn = transform;

		// Initialize particles
		for (unsigned int i = 0; i < maxParticles; i++)
		{
			Particle p;
			p.acc = { 0.0f, 0.0f, 0.0f };
			

			// Apply init functions
			for (const auto& func : initFuncs)
			{
				curr = &p;
				func();
			}

			p.pos += trn->pos;

			// Add particle to list
			particles.push_back(p);
		}
	}
}

unsigned int ParticleEmitter::getNextParticle()
{
	/*
	for (unsigned int i = lastUsedParticle; i < maxParticles; i++) 
	{
		if (particles[i].life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	*/
	

	for (unsigned int i = 0; i < maxParticles; i++)
	{
		if (particles[i].life <= 0.0f) 
		{
			lastUsedParticle = i;
			return i;
		}
	}

	// override first particle if all others are alive
	lastUsedParticle = 0;
	return 0;
}


void ParticleEmitter::spawn(Particle& particle)
{

	// Apply init functions
	for (const auto& func : initFuncs)
	{
		curr = &particle;
		func();
	}

	particle.pos += trn->pos;
}


void ParticleEmitter::update(Engine& engine)
{
	if (state == EmitterState::RUNNING)
	{

		float& time = engine.getWorld().time;
		float& dt = engine.getWorld().time_dx;

		systemTime += dt * spawnRate;


		if (frame % spawnRate == 0)
		{
			unsigned int index = getNextParticle();

			spawn(particles[index]);
		}

		// update all particles
		for (unsigned int i = 0; i < maxParticles; ++i)
		{
			Particle& p = particles[i];

			// Only update if alive
			if (p.life > 0.0f)
			{
				// Apply update functions
				for (const auto& func : updateFuncs)
				{
					curr = &p;
					func();
				}

				p.life -= dt; // Reduce life
				p.pos -= p.vel * dt; // Update position

			}

			p.life = glm::max(p.life, -0.0001f);

		}

		frame++;
	}
}