#include "ParticleSystem.h"
#include "Renderer.h"
#include <iostream>
#include <random>
#include <cmath>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace av {

ParticleSystem::ParticleSystem(int maxParticles)
    : m_maxParticles(maxParticles)
    , m_activeParticles(0)
    , m_gravityX(0.0f)
    , m_gravityY(0.0f)
{
    // Initialize random number generator
    std::random_device rd;
    m_random = std::mt19937(rd());
    
    // Initialize particle array
    m_particles.resize(maxParticles);
    clearParticles();
    
    std::cout << "ParticleSystem initialized with " << maxParticles << " maximum particles" << std::endl;
}

ParticleSystem::~ParticleSystem()
{
    std::cout << "ParticleSystem destroyed" << std::endl;
}

void ParticleSystem::update(float deltaTime)
{
    // In a real implementation, this would update all particles
    // For the placeholder, just decrement life and move particles
    for (size_t i = 0; i < m_particles.size(); ++i) {
        auto& p = m_particles[i];
        
        if (p.active) {
            // Update life
            p.life -= deltaTime;
            
            if (p.life <= 0.0f) {
                p.active = false;
                m_activeParticles--;
                continue;
            }
            
            // Apply gravity
            p.vx += m_gravityX * deltaTime;
            p.vy += m_gravityY * deltaTime;
            
            // Move particle
            p.x += p.vx * deltaTime;
            p.y += p.vy * deltaTime;
        }
    }
}

void ParticleSystem::render(Renderer* renderer)
{
    if (!renderer) return;
    
    // In a real implementation, this would render all particles
    for (const auto& p : m_particles) {
        if (p.active) {
            // Calculate interpolated color based on life
            float lifeFactor = p.life / p.maxLife;
            Color color;
            
            // Linear interpolation between start and end colors
            color.r = p.startColor.r * lifeFactor + p.endColor.r * (1.0f - lifeFactor);
            color.g = p.startColor.g * lifeFactor + p.endColor.g * (1.0f - lifeFactor);
            color.b = p.startColor.b * lifeFactor + p.endColor.b * (1.0f - lifeFactor);
            color.a = p.startColor.a * lifeFactor + p.endColor.a * (1.0f - lifeFactor);
            
            renderer->drawParticle(p.x, p.y, p.size, color, p.shapeType);
        }
    }
}

void ParticleSystem::emit(float x, float y, int count, float minVel, float maxVel, 
                        float minSize, float maxSize, float minLife, float maxLife, 
                        const Color& startColor, const Color& endColor, int shapeType)
{
    std::uniform_real_distribution<float> velDist(minVel, maxVel);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> sizeDist(minSize, maxSize);
    std::uniform_real_distribution<float> lifeDist(minLife, maxLife);
    
    for (int i = 0; i < count; ++i) {
        if (m_activeParticles >= m_maxParticles) {
            break; // No more room for particles
        }
        
        // Find an inactive particle
        for (auto& p : m_particles) {
            if (!p.active) {
                // Initialize the particle
                p.active = true;
                p.x = x;
                p.y = y;
                
                // Random velocity based on angle and speed
                float angle = angleDist(m_random);
                float speed = velDist(m_random);
                p.vx = std::cos(angle) * speed;
                p.vy = std::sin(angle) * speed;
                
                // Random size and life
                p.size = sizeDist(m_random);
                p.maxLife = lifeDist(m_random);
                p.life = p.maxLife;
                p.rotation = angleDist(m_random);
                p.shapeType = shapeType;
                p.startColor = startColor;
                p.endColor = endColor;
                
                m_activeParticles++;
                break;
            }
        }
    }
}

void ParticleSystem::setGravity(float x, float y)
{
    m_gravityX = x;
    m_gravityY = y;
}

void ParticleSystem::clearParticles()
{
    for (auto& p : m_particles) {
        p.active = false;
    }
    m_activeParticles = 0;
}

int ParticleSystem::getActiveParticleCount() const
{
    return m_activeParticles;
}

int ParticleSystem::getMaxParticleCount() const
{
    return m_maxParticles;
}

} // namespace av 