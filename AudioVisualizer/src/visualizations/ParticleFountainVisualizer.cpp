#include "visualizations/ParticleFountainVisualizer.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SDL.h>
#include <GL/glew.h>

namespace av {

ParticleFountainVisualizer::ParticleFountainVisualizer()
    : Visualization("Particle Fountain")
    , m_maxParticles(2000)
    , m_emissionRate(300.0f)  // Particles per second
    , m_particleSize(5.0f)
    , m_gravity(400.0f)
    , m_lastUpdateTime(0.0f)
    , m_fountainX(0.5f)
    , m_fountainY(0.8f)  // Start from bottom of screen
    , m_fountainWidth(0.3f)
    , m_fountainHeight(0.1f)
    , m_fountainSpread(30.0f)
    , m_fountainBaseVelocity(400.0f)
{
    std::cout << "ParticleFountainVisualizer created" << std::endl;
    
    // Initialize random number generator
    std::random_device rd;
    m_rng = std::mt19937(rd());
    
    // Pre-allocate particle buffer
    m_particles.resize(m_maxParticles);
    for (auto& p : m_particles) {
        p.active = false;
    }
}

ParticleFountainVisualizer::~ParticleFountainVisualizer()
{
    cleanup();
}

void ParticleFountainVisualizer::cleanup()
{
    // Nothing to clean up
}

void ParticleFountainVisualizer::onResize(int width, int height)
{
    // Nothing specific to do here
}

void ParticleFountainVisualizer::updateParticles(const AudioData& audioData, float deltaTime)
{
    // Update physics and life of each particle
    for (auto& p : m_particles) {
        if (!p.active) {
            continue;
        }
        
        // Update position based on velocity
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        
        // Apply gravity
        p.vy += m_gravity * deltaTime;
        
        // Apply slight dampening
        p.vx *= 0.99f;
        
        // Decrease life
        p.life -= deltaTime;
        
        // Deactivate if life is over or particle is offscreen
        if (p.life <= 0.0f) {
            p.active = false;
        }
    }
}

void ParticleFountainVisualizer::emitParticles(const AudioData& audioData, float deltaTime)
{
    // Calculate number of particles to emit based on audio levels and time
    float emissionMultiplier = 1.0f + audioData.energy * 3.0f;
    float emitCount = m_emissionRate * emissionMultiplier * deltaTime;
    
    // Add a burst when there's a beat
    static float lastBass = 0.0f;
    float bassDelta = audioData.bass - lastBass;
    if (bassDelta > 0.2f) {
        // Bass hit detected, add a burst of particles
        emitCount += bassDelta * 100.0f;
    }
    lastBass = audioData.bass;
    
    // Integer part determines definite emissions
    int definiteEmissions = static_cast<int>(emitCount);
    
    // Fractional part determines a chance for one more
    float fractionalPart = emitCount - definiteEmissions;
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
    if (chanceDist(m_rng) < fractionalPart) {
        definiteEmissions += 1;
    }
    
    // Distribution for randomizing particle properties
    std::uniform_real_distribution<float> posDist(-m_fountainWidth / 2, m_fountainWidth / 2);
    std::uniform_real_distribution<float> angleDist(-m_fountainSpread, m_fountainSpread);
    std::uniform_real_distribution<float> speedDist(0.8f, 1.2f);
    std::uniform_real_distribution<float> sizeDist(0.7f, 1.3f);
    std::uniform_real_distribution<float> lifeDist(0.8f, 1.2f);
    
    // Find inactive particles to reuse
    int emitted = 0;
    for (auto& p : m_particles) {
        if (emitted >= definiteEmissions) {
            break;
        }
        
        if (!p.active) {
            // Initialize this particle
            p.active = true;
            
            // Position (fountain source with some spread)
            p.x = m_fountainX + posDist(m_rng);
            p.y = m_fountainY;
            
            // Calculate base velocity - higher for treble, lower for bass
            float baseVelocity = m_fountainBaseVelocity * (0.8f + audioData.energy * 0.7f);
            
            // Direction - upward with spread
            float angle = -90.0f + angleDist(m_rng); // -90 is straight up
            float angleRad = angle * 3.14159f / 180.0f;
            float speed = baseVelocity * speedDist(m_rng);
            
            // Velocity components
            p.vx = std::cos(angleRad) * speed;
            p.vy = std::sin(angleRad) * speed;
            
            // Size varies with audio frequency
            float sizeMultiplier = 1.0f;
            if (emitted % 3 == 0) {
                // Bass particles - larger
                p.size = m_particleSize * 1.5f * sizeDist(m_rng);
                p.hue = 0.0f + audioData.bass * 0.1f; // Red-orange
            } else if (emitted % 3 == 1) {
                // Mid particles - medium
                p.size = m_particleSize * sizeDist(m_rng);
                p.hue = 0.33f + audioData.mid * 0.1f; // Green-yellow
            } else {
                // Treble particles - smaller but more
                p.size = m_particleSize * 0.7f * sizeDist(m_rng);
                p.hue = 0.6f + audioData.treble * 0.1f; // Blue-purple
            }
            
            // Lifespan
            p.maxLife = 1.0f + audioData.energy * 0.5f;
            p.maxLife *= lifeDist(m_rng);
            p.life = p.maxLife;
            
            emitted++;
        }
    }
}

void ParticleFountainVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // Set fountain position relative to window size
    m_fountainX = width / 2;
    m_fountainY = height * 0.8f;
    m_fountainWidth = width * 0.3f;
    
    // Calculate delta time for physics
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - m_lastUpdateTime) / 1000.0f;
    m_lastUpdateTime = currentTime;
    
    // Clamp delta time to avoid large steps
    deltaTime = std::min(deltaTime, 0.05f);
    
    // Track frame count for animations
    static int frameCount = 0;
    frameCount++;
    
    // Create a gradient background that shifts with the music
    float bgHue = frameCount * 0.001f;
    Color topColor = Color::fromHSV(bgHue, 0.7f, 0.2f);
    Color bottomColor = Color::fromHSV(bgHue + 0.5f, 0.7f, 0.1f);
    
    // Draw gradient background
    glBegin(GL_QUADS);
    glColor4f(topColor.r, topColor.g, topColor.b, topColor.a);
    glVertex2f(0, 0);                      // Top left
    glVertex2f(width, 0);                  // Top right
    glColor4f(bottomColor.r, bottomColor.g, bottomColor.b, bottomColor.a);
    glVertex2f(width, height);             // Bottom right
    glVertex2f(0, height);                 // Bottom left
    glEnd();
    
    // Enable blending for particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // Update particle physics
    updateParticles(audioData, deltaTime);
    
    // Emit new particles
    emitParticles(audioData, deltaTime);
    
    // Draw particles
    for (const auto& p : m_particles) {
        if (!p.active) {
            continue;
        }
        
        // Calculate alpha based on remaining life
        float alpha = p.life / p.maxLife;
        alpha = std::pow(alpha, 0.5f); // Make fade more gradual
        
        // Calculate color based on hue, with full saturation and brightness
        Color color = Color::fromHSV(p.hue, 0.8f, 1.0f, alpha);
        
        // Draw the particle as a circle
        float screenX = p.x;
        float screenY = p.y;
        
        renderer->drawFilledCircle(screenX, screenY, p.size, color);
        
        // Add a glow effect with a larger, more transparent circle
        Color glowColor = Color::fromHSV(p.hue, 0.7f, 0.9f, alpha * 0.5f);
        renderer->drawFilledCircle(screenX, screenY, p.size * 2.0f, glowColor);
    }
    
    // Disable blending when done
    glDisable(GL_BLEND);
    
    // Draw the fountain source - a rectangle at the bottom with glow
    float fountainGlow = 0.5f + audioData.energy * 0.5f;
    Color fountainColor = Color::fromHSV(0.5f, 0.7f, fountainGlow);
    
    // Main fountain
    renderer->drawFilledRect(
        m_fountainX - m_fountainWidth / 2,
        m_fountainY - m_fountainHeight,
        m_fountainWidth,
        m_fountainHeight,
        fountainColor
    );
    
    // Glow around the fountain
    float glowSize = m_fountainWidth * 0.2f * (1.0f + audioData.energy);
    Color fountainGlowColor = Color::fromHSV(0.5f, 0.7f, fountainGlow * 0.7f, 0.7f);
    renderer->drawRect(
        m_fountainX - m_fountainWidth / 2 - glowSize,
        m_fountainY - m_fountainHeight - glowSize,
        m_fountainWidth + glowSize * 2,
        m_fountainHeight + glowSize * 2,
        fountainGlowColor,
        glowSize
    );
    
    // Add audio frequency visualization at bottom
    if (!audioData.spectrum.empty()) {
        // Draw a spectrum analyzer at the bottom
        float spectrumHeight = 30.0f;
        float spectrumY = height - spectrumHeight - 10;
        
        // Use a subset of the spectrum
        int barCount = std::min(64, static_cast<int>(audioData.spectrum.size()));
        float barWidth = width / barCount;
        
        for (int i = 0; i < barCount; i++) {
            // Get frequency value
            float value = audioData.spectrum[i * audioData.spectrum.size() / barCount];
            value = std::min(1.0f, value * 3.0f); // Amplify
            
            // Height based on value
            float barHeight = value * spectrumHeight;
            
            // Position
            float x = i * barWidth;
            float y = spectrumY + spectrumHeight - barHeight;
            
            // Color based on frequency
            float hue = static_cast<float>(i) / barCount;
            Color barColor = Color::fromHSV(hue, 0.8f, 0.9f, 0.7f);
            
            // Draw bar
            renderer->drawFilledRect(x, y, barWidth - 1, barHeight, barColor);
        }
    }
}

std::string ParticleFountainVisualizer::getDescription() const
{
    return "Audio-reactive particle fountain";
}

} // namespace av 