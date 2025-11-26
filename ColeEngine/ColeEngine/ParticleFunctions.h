#pragma once


#ifndef _PARTICLEFUNCTIONS
#define _PARTICLEFUNCTIONS

#include <functional>
#include <glm/glm.hpp>

#include"ParticleEmitter.h"

class ParticleEmitter;

// Particle init functions
void setLifeTimeRandom(Particle* p, float lower, float upper);
void setLifeTime(Particle* p, float life);
void setColor(Particle* p, glm::vec4 col);
void setColorRandom(Particle* p);

void setVelocityRandom(Particle* p, glm::vec3 low, glm::vec3 hi);
void setVelocityRandom(Particle* p, float low, float hi);

void setVelocity(Particle* p, glm::vec3 vel);
void setPositionSphere(Particle* p, float radius);


// Particle update functions
void addVelocity(Particle* p, glm::vec3 vel);
void multiplyScale(Particle* p, glm::vec3 scl);
void multiplyScale(Particle* p, float scl);
void setScaleOverLife(Particle* p, glm::vec3 scl0, glm::vec3 scl1);
void setScaleOverLife(Particle* p, float scl0, float scl1);
void multiplyVelocity(Particle* p, glm::vec3 vel);
void setColorOverLife(Particle* p, glm::vec4 col0, glm::vec4 col1);
void setAlphaOverLife(Particle* p, float low, float hi);




#endif