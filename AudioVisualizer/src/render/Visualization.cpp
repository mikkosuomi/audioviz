#include "Visualization.h"
#include "Window.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace av {

// VisualizationManager implementation
VisualizationManager::VisualizationManager()
    : m_currentVisualization(nullptr)
    , m_currentIndex(0)
{
    // Add all available visualizations
    m_visualizations.push_back(std::make_unique<SpectrumVisualization>());
    m_visualizations.push_back(std::make_unique<WaveformVisualization>());
    m_visualizations.push_back(std::make_unique<CircularVisualization>());
    m_visualizations.push_back(std::make_unique<ParticleVisualization>());
    
    // Set default visualization
    if (!m_visualizations.empty()) {
        m_currentVisualization = m_visualizations[0].get();
    }
}

void VisualizationManager::selectVisualization(int index)
{
    if (index >= 0 && index < static_cast<int>(m_visualizations.size())) {
        m_currentIndex = index;
        m_currentVisualization = m_visualizations[index].get();
        std::cout << "Selected visualization: " << m_currentVisualization->getName() << std::endl;
    }
}

void VisualizationManager::nextVisualization()
{
    int nextIndex = (m_currentIndex + 1) % m_visualizations.size();
    selectVisualization(nextIndex);
}

void VisualizationManager::prevVisualization()
{
    int prevIndex = (m_currentIndex - 1 + m_visualizations.size()) % m_visualizations.size();
    selectVisualization(prevIndex);
}

// Spectrum Visualization implementation
void SpectrumVisualization::render(Renderer* renderer, const AudioData& audioData, float time)
{
    if (!renderer) return;
    
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // Draw background
    Color bgColor = Color::fromHSV(time * 0.05f, 0.2f, 0.2f);
    renderer->drawFilledRect(0, 0, width, height, bgColor);
    
    // Draw spectrum bars
    const std::vector<float>& spectrum = audioData.spectrum;
    int numBars = std::min(static_cast<int>(spectrum.size()), 100);
    float barWidth = width / static_cast<float>(numBars);
    
    for (int i = 0; i < numBars; i++) {
        float value = spectrum[i * spectrum.size() / numBars];
        
        // Add some visual enhancement
        value = std::pow(value, 0.8f); // Less extreme values
        value = std::min(value * 1.5f, 1.0f); // Boost display value
        
        float barHeight = value * height * 0.8f;
        float x = i * barWidth;
        float y = height - barHeight;
        
        // Base color on frequency
        float hue = static_cast<float>(i) / numBars * 0.8f + time * 0.1f;
        Color barColor = Color::fromHSV(hue, 0.7f + value * 0.3f, 0.8f + value * 0.2f);
        
        // Draw bar with slight gap
        renderer->drawFilledRect(x, y, barWidth * 0.9f, barHeight, barColor);
    }
    
    // Draw audio energy indicators
    float bassRadius = 30.0f + audioData.bass * 60.0f;
    float midRadius = 20.0f + audioData.mid * 40.0f;
    float trebleRadius = 10.0f + audioData.treble * 20.0f;
    
    renderer->drawFilledCircle(width * 0.1f, height * 0.15f, bassRadius, 
                              Color::fromHSV(time * 0.07f, 0.8f, 0.8f, 0.7f));
    renderer->drawFilledCircle(width * 0.9f, height * 0.15f, midRadius, 
                              Color::fromHSV(time * 0.07f + 0.33f, 0.8f, 0.8f, 0.7f));
    renderer->drawFilledCircle(width * 0.5f, height * 0.15f, trebleRadius, 
                              Color::fromHSV(time * 0.07f + 0.66f, 0.8f, 0.8f, 0.7f));
}

// Waveform Visualization implementation
void WaveformVisualization::render(Renderer* renderer, const AudioData& audioData, float time)
{
    if (!renderer) return;
    
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // Draw background
    Color bgColor = Color::fromHSV(time * 0.05f + 0.3f, 0.2f, 0.2f);
    renderer->drawFilledRect(0, 0, width, height, bgColor);
    
    // Draw grid lines
    Color gridColor(0.3f, 0.3f, 0.3f, 0.5f);
    int gridCount = 10;
    
    for (int i = 1; i < gridCount; i++) {
        float y = height * i / gridCount;
        renderer->drawLine(0, y, width, y, gridColor, 1.0f);
        
        float x = width * i / gridCount;
        renderer->drawLine(x, 0, x, height, gridColor, 1.0f);
    }
    
    // Draw center line
    Color centerLineColor(0.5f, 0.5f, 0.5f, 0.7f);
    renderer->drawLine(0, height / 2, width, height / 2, centerLineColor, 2.0f);
    
    // Draw waveform
    const std::vector<float>& waveform = audioData.waveform;
    int numPoints = waveform.size();
    
    // Use different colors for waveform based on audio characteristics
    float hue = 0.6f + 0.2f * audioData.energy + time * 0.05f;
    Color waveColor = Color::fromHSV(hue, 0.8f, 0.9f);
    
    // Draw waveform in main area
    renderer->drawWaveform(waveform.data(), waveform.size(), 0, height / 2, width, height / 2, waveColor);
    
    // Overlay audio levels as a horizontal bar
    float barWidth = width * 0.8f;
    float barHeight = 20.0f;
    float barX = (width - barWidth) / 2;
    float barY = height - barHeight - 20.0f;
    
    renderer->drawRect(barX, barY, barWidth, barHeight, Color(0.7f, 0.7f, 0.7f, 0.5f));
    
    // Draw bass, mid, and treble levels
    float bassWidth = barWidth * 0.33f * audioData.bass;
    float midWidth = barWidth * 0.33f * audioData.mid;
    float trebleWidth = barWidth * 0.33f * audioData.treble;
    
    renderer->drawFilledRect(barX, barY, bassWidth, barHeight, 
                           Color::fromHSV(0.0f, 0.8f, 0.8f, 0.8f));
    renderer->drawFilledRect(barX + barWidth * 0.33f, barY, midWidth, barHeight, 
                           Color::fromHSV(0.3f, 0.8f, 0.8f, 0.8f));
    renderer->drawFilledRect(barX + barWidth * 0.66f, barY, trebleWidth, barHeight, 
                           Color::fromHSV(0.6f, 0.8f, 0.8f, 0.8f));
}

// Circular Visualization implementation
void CircularVisualization::render(Renderer* renderer, const AudioData& audioData, float time)
{
    if (!renderer) return;
    
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    
    // Draw background
    Color bgColor = Color::fromHSV(time * 0.05f + 0.6f, 0.2f, 0.2f);
    renderer->drawFilledRect(0, 0, width, height, bgColor);
    
    // Calculate base radius based on window size
    float maxRadius = std::min(width, height) * 0.4f;
    
    // Draw circular spectrum
    const std::vector<float>& spectrum = audioData.spectrum;
    int numSegments = std::min(static_cast<int>(spectrum.size()), 180);
    
    // Draw outer circle
    renderer->drawCircle(centerX, centerY, maxRadius, Color(0.5f, 0.5f, 0.5f, 0.3f));
    
    // Draw frequency segments
    for (int i = 0; i < numSegments; i++) {
        float angle = i * 2.0f * M_PI / numSegments;
        float nextAngle = (i + 1) * 2.0f * M_PI / numSegments;
        
        // Get spectrum value for this segment
        float value = spectrum[i * spectrum.size() / numSegments];
        value = std::min(value * 1.2f, 1.0f); // Boost display value
        
        // Calculate inner and outer radii
        float innerRadius = maxRadius * 0.4f;
        float outerRadius = innerRadius + maxRadius * 0.6f * value;
        
        // Calculate points for the segment
        float x1 = centerX + std::cos(angle) * innerRadius;
        float y1 = centerY + std::sin(angle) * innerRadius;
        float x2 = centerX + std::cos(angle) * outerRadius;
        float y2 = centerY + std::sin(angle) * outerRadius;
        float x3 = centerX + std::cos(nextAngle) * outerRadius;
        float y3 = centerY + std::sin(nextAngle) * outerRadius;
        float x4 = centerX + std::cos(nextAngle) * innerRadius;
        float y4 = centerY + std::sin(nextAngle) * innerRadius;
        
        // Draw segment as a filled polygon
        float points[] = { x1, y1, x2, y2, x3, y3, x4, y4 };
        
        // Color based on frequency and time
        float hue = static_cast<float>(i) / numSegments + time * 0.1f;
        Color segmentColor = Color::fromHSV(hue, 0.7f + value * 0.3f, 0.5f + value * 0.5f, 0.8f);
        
        renderer->drawFilledPolygon(points, 8, segmentColor);
    }
    
    // Draw reactive circles based on audio energy
    float bassRadius = maxRadius * 0.3f * (0.5f + audioData.bass * 0.5f);
    float midRadius = maxRadius * 0.2f * (0.5f + audioData.mid * 0.5f);
    float trebleRadius = maxRadius * 0.1f * (0.5f + audioData.treble * 0.5f);
    
    // Draw with slight transparency for layering effect
    renderer->drawFilledCircle(centerX, centerY, bassRadius, 
                              Color::fromHSV(time * 0.07f, 0.7f, 0.7f, 0.4f));
    renderer->drawFilledCircle(centerX, centerY, midRadius, 
                              Color::fromHSV(time * 0.07f + 0.33f, 0.7f, 0.8f, 0.5f));
    renderer->drawFilledCircle(centerX, centerY, trebleRadius, 
                              Color::fromHSV(time * 0.07f + 0.66f, 0.7f, 0.9f, 0.6f));
    
    // Add rotating element based on beat detection
    float rotationAngle = time * 0.5f;
    float beatRadius = maxRadius * 1.1f * (0.2f + audioData.transient * 0.8f);
    
    for (int i = 0; i < 6; i++) {
        float angle = rotationAngle + i * M_PI / 3.0f;
        float x = centerX + std::cos(angle) * beatRadius;
        float y = centerY + std::sin(angle) * beatRadius;
        
        // Size based on audio energy
        float size = 10.0f + 30.0f * audioData.energy;
        
        renderer->drawFilledCircle(x, y, size, 
                                 Color::fromHSV(angle * 0.1f + time * 0.1f, 0.9f, 0.9f, 0.7f));
    }
}

// Particle Visualization implementation
ParticleVisualization::ParticleVisualization()
    : Visualization("Particles")
    , m_maxParticles(200)
    , m_lastTime(0.0f)
    , m_width(800)
    , m_height(600)
{
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(0, m_width);
    std::uniform_real_distribution<float> yDist(0, m_height);
    std::uniform_real_distribution<float> sizeDist(5, 15);
    std::uniform_real_distribution<float> hueDist(0, 1);
    std::uniform_int_distribution<int> typeDist(0, 5);
    std::uniform_real_distribution<float> lifeDist(1, 5);
    
    // Create initial particles
    for (int i = 0; i < m_maxParticles / 2; i++) {
        Particle p;
        p.x = xDist(gen);
        p.y = yDist(gen);
        p.vx = 0;
        p.vy = 0;
        p.size = sizeDist(gen);
        p.life = lifeDist(gen);
        p.maxLife = p.life;
        p.hue = hueDist(gen);
        p.type = typeDist(gen);
        
        m_particles.push_back(p);
    }
}

void ParticleVisualization::render(Renderer* renderer, const AudioData& audioData, float time)
{
    if (!renderer) return;
    
    // Update window dimensions
    m_width = renderer->getWidth();
    m_height = renderer->getHeight();
    
    // Calculate delta time
    float deltaTime = 0.016f; // Default to ~60fps
    if (m_lastTime > 0) {
        deltaTime = time - m_lastTime;
    }
    m_lastTime = time;
    
    // Draw background with trail effect
    Color bgColor = Color::fromHSV(time * 0.02f + 0.7f, 0.3f, 0.1f, 0.2f);
    renderer->drawFilledRect(0, 0, m_width, m_height, bgColor);
    
    // Update and spawn particles
    updateParticles(audioData, deltaTime);
    spawnParticles(audioData, time);
    
    // Render particles
    renderParticles(renderer);
    
    // Draw audio waveform at the bottom
    const std::vector<float>& waveform = audioData.waveform;
    Color waveColor = Color::fromHSV(time * 0.1f + 0.5f, 0.8f, 0.8f, 0.6f);
    renderer->drawWaveform(waveform.data(), waveform.size(), 
                          0, m_height * 0.85f, m_width, m_height * 0.15f, waveColor);
}

void ParticleVisualization::updateParticles(const AudioData& audioData, float deltaTime)
{
    // Audio-influenced parameters
    float speedFactor = 1.0f + audioData.energy * 3.0f;
    float sizeFactor = 1.0f + audioData.bass * 0.5f;
    
    // Center of the screen for force calculations
    float centerX = m_width / 2.0f;
    float centerY = m_height / 2.0f;
    
    // Update all particles
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        Particle& p = *it;
        
        // Decrease life
        p.life -= deltaTime;
        
        if (p.life <= 0) {
            // Remove dead particles
            it = m_particles.erase(it);
            continue;
        }
        
        // Calculate life ratio for effects
        float lifeRatio = p.life / p.maxLife;
        
        // Apply audio-reactive forces
        // Direction toward/away from center based on frequencies
        float dx = p.x - centerX;
        float dy = p.y - centerY;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist > 1.0f) {
            float angle = std::atan2(dy, dx);
            
            // Different frequencies affect different behaviors
            float bassForce = audioData.bass * 100.0f * deltaTime;
            float midForce = audioData.mid * 50.0f * deltaTime;
            float trebleForce = audioData.treble * 20.0f * deltaTime;
            
            // Bass pushes outward, treble pulls inward, mid causes rotation
            p.vx += std::cos(angle) * bassForce - std::cos(angle) * trebleForce - std::sin(angle) * midForce;
            p.vy += std::sin(angle) * bassForce - std::sin(angle) * trebleForce + std::cos(angle) * midForce;
        }
        
        // Add some randomness on beat
        if (audioData.transient > 0.2f) {
            p.vx += (std::rand() / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * audioData.transient * 10.0f * deltaTime;
            p.vy += (std::rand() / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * audioData.transient * 10.0f * deltaTime;
        }
        
        // Apply drag (proportional to speed and inverse to size)
        float drag = 0.98f;
        p.vx *= drag;
        p.vy *= drag;
        
        // Update position
        p.x += p.vx * speedFactor * deltaTime * 60.0f;
        p.y += p.vy * speedFactor * deltaTime * 60.0f;
        
        // Apply size based on audio
        p.size = p.size * sizeFactor;
        
        // Handle boundaries (wrap around)
        if (p.x < -p.size) p.x = m_width + p.size;
        if (p.x > m_width + p.size) p.x = -p.size;
        if (p.y < -p.size) p.y = m_height + p.size;
        if (p.y > m_height + p.size) p.y = -p.size;
        
        // Move to next particle
        ++it;
    }
}

void ParticleVisualization::spawnParticles(const AudioData& audioData, float time)
{
    // Only spawn particles if we have room and audio has energy
    if (m_particles.size() >= m_maxParticles || audioData.energy < 0.05f) {
        return;
    }
    
    // Audio-based spawning
    int spawnCount = static_cast<int>(audioData.transient * 20.0f);
    
    // Spawn based on bass, mid, treble energy
    if (audioData.bass > 0.6f) spawnCount += 3;
    if (audioData.mid > 0.6f) spawnCount += 2;
    if (audioData.treble > 0.6f) spawnCount += 1;
    
    // Limit new particles per frame
    spawnCount = std::min(spawnCount, 10);
    
    // Center of the screen for force calculations
    float centerX = m_width / 2.0f;
    float centerY = m_height / 2.0f;
    
    for (int i = 0; i < spawnCount; i++) {
        if (m_particles.size() >= m_maxParticles) {
            break;
        }
        
        Particle p;
        
        // Spawn from center
        p.x = centerX;
        p.y = centerY;
        
        // Initial velocity based on audio
        float angle = time * 2.0f + i * (2.0f * M_PI / spawnCount);
        float speed = 5.0f + audioData.energy * 20.0f;
        
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        
        // Size based on frequency components
        float sizeFactor = 0.5f;
        if (i % 3 == 0) {
            // Bass particles are larger
            p.size = 15.0f + audioData.bass * 20.0f;
            p.hue = time * 0.1f;
            sizeFactor = 0.7f;
        } else if (i % 3 == 1) {
            // Mid particles
            p.size = 10.0f + audioData.mid * 15.0f;
            p.hue = time * 0.1f + 0.33f;
            sizeFactor = 1.0f;
        } else {
            // Treble particles are smaller
            p.size = 5.0f + audioData.treble * 10.0f;
            p.hue = time * 0.1f + 0.66f;
            sizeFactor = 1.3f;
        }
        
        // Life based on size (smaller particles live longer)
        p.maxLife = 2.0f + (15.0f / p.size) * sizeFactor;
        p.life = p.maxLife;
        
        // Particle shape
        p.type = i % 6;
        
        m_particles.push_back(p);
    }
}

void ParticleVisualization::renderParticles(Renderer* renderer)
{
    for (const auto& p : m_particles) {
        // Alpha based on life
        float alpha = p.life / p.maxLife;
        
        // Color based on hue
        Color color = Color::fromHSV(p.hue, 0.9f, 0.9f, alpha);
        
        // Draw particle
        renderer->drawParticle(p.x, p.y, p.size, color, p.type);
    }
}

} // namespace av 