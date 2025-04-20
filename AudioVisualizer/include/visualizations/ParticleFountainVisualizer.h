#pragma once

#include "../Visualization.h"
#include <vector>
#include <random>

namespace av {

class ParticleFountainVisualizer : public Visualization {
public:
    ParticleFountainVisualizer();
    virtual ~ParticleFountainVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);

private:
    struct Particle {
        float x, y;          // Position
        float vx, vy;        // Velocity
        float size;          // Size of the particle
        float life;          // Current life (decreases over time)
        float maxLife;       // Maximum life span
        float hue;           // Color hue
        bool active;         // Whether the particle is active
    };

    void updateParticles(const AudioData& audioData, float deltaTime);
    void emitParticles(const AudioData& audioData, float deltaTime);
    
    std::vector<Particle> m_particles;
    std::mt19937 m_rng;
    
    int m_maxParticles;
    float m_emissionRate;
    float m_particleSize;
    float m_gravity;
    float m_lastUpdateTime;
    
    // Fountain parameters
    float m_fountainX;
    float m_fountainY;
    float m_fountainWidth;
    float m_fountainHeight;
    float m_fountainSpread;
    float m_fountainBaseVelocity;
};

} // namespace av 