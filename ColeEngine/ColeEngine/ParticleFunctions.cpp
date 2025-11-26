
#include "ParticleFunctions.h"

static glm::vec3 randomPointInSphere(float radius)
{
	// Generate random spherical coordinates
	float theta = glm::linearRand(0.0f, glm::pi<float>() * 2.0f);   // Azimuthal angle
	float phi = glm::acos(glm::linearRand(-1.0f, 1.0f));             // Polar angle

	// Convert spherical coordinates to Cartesian coordinates
	float x = radius * glm::sin(phi) * glm::cos(theta);
	float y = radius * glm::sin(phi) * glm::sin(theta);
	float z = radius * glm::cos(phi);

	return glm::vec3(x, y, z);
}

void setLifeTimeRandom(Particle* p, float lower, float upper)
{
	p->life = glm::linearRand(lower, upper);
	p->lifeInit = p->life;
}

void setLifeTime(Particle* p, float life)
{
	p->life = life;
	p->lifeInit = life;
}

void setColor(Particle* p, glm::vec4 col)
{
	p->col = col;
}

void setColorRandom(Particle* p)
{
	float r = glm::linearRand(0.0f, 1.0f);
	float g = glm::linearRand(0.0f, 1.0f);
	float b = glm::linearRand(0.0f, 1.0f);

	p->col = glm::vec4(r, g, b, 1.0f);
}

void setVelocityRandom(Particle* p, glm::vec3 low, glm::vec3 hi)
{
	float x = glm::linearRand(low.x, hi.x);
	float y = glm::linearRand(low.y, hi.y);
	float z = glm::linearRand(low.z, hi.z);

	p->vel = glm::vec3(x, y, z);
}

void setVelocityRandom(Particle* p, float low, float hi)
{
	float x = glm::linearRand(low, hi);
	float y = glm::linearRand(low, hi);
	float z = glm::linearRand(low, hi);

	p->vel = glm::vec3(x, y, z);
}

void setVelocity(Particle* p, glm::vec3 vel)
{
	p->vel = vel;
}

void setPositionSphere(Particle* p, float radius)
{
	p->pos = randomPointInSphere(radius);
}

// Update



void addVelocity(Particle* p, glm::vec3 vel)
{
	p->vel += vel;
}

void multiplyScale(Particle* p, glm::vec3 scl)
{
	p->scale *= scl;
}

void multiplyVelocity(Particle* p, glm::vec3 vel)
{
	p->vel *= vel;
}

void multiplyScale(Particle* p, float scl)
{
	p->scale *= scl;
}

void setScaleOverLife(Particle* p, glm::vec3 scl0, glm::vec3 scl1)
{
	float x = 1.0f - (p->life / p->lifeInit);

	p->scale = glm::lerp(scl0, scl1, x);
}

void setScaleOverLife(Particle* p, float scl0, float scl1)
{
	float x = 1.0f - (p->life / p->lifeInit);

	p->scale = glm::lerp(glm::vec3(scl0, scl0, scl0), glm::vec3(scl1, scl1, scl1), x);
}

void setColorOverLife(Particle* p, glm::vec4 col0, glm::vec4 col1)
{
	float x = 1.0f - (p->life / p->lifeInit);

	p->col = glm::lerp(col0, col1, x);
}

void setAlphaOverLife(Particle* p, float low, float hi)
{
	float x = 1.0f - (p->life / p->lifeInit);

	p->col.w = glm::lerp(low, hi, x);
}