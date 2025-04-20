#pragma once

#include "Renderer.h"
#include <vector>
#include <random>

namespace av {

class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 1000);
    ~ParticleSystem();
    
    void update(float deltaTime);
    void render(Renderer* renderer);
    
    // Particle emission
    void emit(float x, float y, int count, float minVel, float maxVel,
              float minSize, float maxSize, float minLife, float maxLife,
              const Color& startColor, const Color& endColor, int shapeType = 0);
    
    void setGravity(float x, float y);
    void clearParticles();
    int getActiveParticleCount() const;
    int getMaxParticleCount() const;
    
private:
    struct Particle {
        float x, y;           // Position
        float vx, vy;         // Velocity
        float size;           // Size
        float life;           // Current life
        float maxLife;        // Maximum life
        float rotation;       // Rotation angle
        int shapeType;        // Shape type (0=circle, 1=square, etc.)
        Color startColor;     // Starting color
        Color endColor;       // Ending color
        bool active;          // Is this particle active?
    };
    
    std::vector<Particle> m_particles;
    std::mt19937 m_random;
    
    int m_maxParticles;
    int m_activeParticles;
    
    float m_gravityX;
    float m_gravityY;
};

} // namespace av 