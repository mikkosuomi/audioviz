#include "Visualization.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

namespace av {

// Base Visualization class implementation
Visualization::Visualization(const std::string& name)
    : m_name(name)
    , m_initialized(false)
    , m_amplificationFactor(20.0f)
{
}

// SpectrumVisualization implementation
SpectrumVisualization::SpectrumVisualization() : Visualization("Spectrum") {
}

void SpectrumVisualization::render(Renderer* renderer, const AudioData& audioData) {
    const float barWidth = 10.0f;
    const float barSpacing = 2.0f;
    const float maxBarHeight = 300.0f;
    
    int windowWidth, windowHeight;
    renderer->getWindowSize(windowWidth, windowHeight);
    
    float startX = windowWidth / 2.0f - (audioData.frequencyData.size() * (barWidth + barSpacing)) / 2.0f;
    
    renderer->setColor(0.2f, 0.6f, 1.0f, 0.8f);
    
    for (size_t i = 0; i < audioData.frequencyData.size(); i++) {
        float barHeight = audioData.frequencyData[i] * maxBarHeight;
        
        float x = startX + i * (barWidth + barSpacing);
        float y = windowHeight / 2.0f - barHeight / 2.0f;
        
        renderer->fillRect(x, y, barWidth, barHeight);
    }
}

// WaveformVisualization implementation
WaveformVisualization::WaveformVisualization() : Visualization("Waveform") {
}

void WaveformVisualization::render(Renderer* renderer, const AudioData& audioData) {
    int windowWidth, windowHeight;
    renderer->getWindowSize(windowWidth, windowHeight);
    
    const float lineThickness = 2.0f;
    const float amplitude = windowHeight * 0.25f;
    
    renderer->setColor(0.0f, 0.8f, 0.4f, 0.8f);
    
    float xStep = static_cast<float>(windowWidth) / audioData.waveformData.size();
    float centerY = windowHeight / 2.0f;
    
    for (size_t i = 0; i < audioData.waveformData.size() - 1; i++) {
        float x1 = i * xStep;
        float y1 = centerY + audioData.waveformData[i] * amplitude;
        float x2 = (i + 1) * xStep;
        float y2 = centerY + audioData.waveformData[i + 1] * amplitude;
        
        renderer->drawLine(x1, y1, x2, y2, lineThickness);
    }
}

// CircularVisualization implementation
CircularVisualization::CircularVisualization() : Visualization("Circular") {
}

void CircularVisualization::render(Renderer* renderer, const AudioData& audioData) {
    int windowWidth, windowHeight;
    renderer->getWindowSize(windowWidth, windowHeight);
    
    const float baseRadius = std::min(windowWidth, windowHeight) * 0.3f;
    const float maxRadiusOffset = 100.0f;
    const int numPoints = std::min(64, static_cast<int>(audioData.frequencyData.size()));
    
    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    
    for (int i = 0; i < numPoints; i++) {
        float angle = 2.0f * 3.14159f * i / numPoints;
        float radiusOffset = audioData.frequencyData[i % audioData.frequencyData.size()] * maxRadiusOffset;
        float radius = baseRadius + radiusOffset;
        
        float hue = static_cast<float>(i) / numPoints;
        float r, g, b;
        
        // Simple HSV to RGB conversion
        if (hue < 1.0f/6.0f) {
            r = 1.0f;
            g = hue * 6.0f;
            b = 0.0f;
        } else if (hue < 2.0f/6.0f) {
            r = 1.0f - (hue - 1.0f/6.0f) * 6.0f;
            g = 1.0f;
            b = 0.0f;
        } else if (hue < 3.0f/6.0f) {
            r = 0.0f;
            g = 1.0f;
            b = (hue - 2.0f/6.0f) * 6.0f;
        } else if (hue < 4.0f/6.0f) {
            r = 0.0f;
            g = 1.0f - (hue - 3.0f/6.0f) * 6.0f;
            b = 1.0f;
        } else if (hue < 5.0f/6.0f) {
            r = (hue - 4.0f/6.0f) * 6.0f;
            g = 0.0f;
            b = 1.0f;
        } else {
            r = 1.0f;
            g = 0.0f;
            b = 1.0f - (hue - 5.0f/6.0f) * 6.0f;
        }
        
        renderer->setColor(r, g, b, 0.7f);
        
        float x1 = centerX + std::cos(angle) * baseRadius;
        float y1 = centerY + std::sin(angle) * baseRadius;
        float x2 = centerX + std::cos(angle) * radius;
        float y2 = centerY + std::sin(angle) * radius;
        
        renderer->drawLine(x1, y1, x2, y2, 4.0f);
    }
}

// ParticleVisualization implementation
ParticleVisualization::ParticleVisualization() : Visualization("Particles") {
    m_particles.resize(1000);
    
    // Initialize particles
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> lifeDist(0.1f, 2.0f);
    
    for (auto& p : m_particles) {
        p.x = posDist(gen);
        p.y = posDist(gen);
        p.vx = posDist(gen) * 0.5f;
        p.vy = posDist(gen) * 0.5f;
        p.life = lifeDist(gen);
        p.color[0] = 0.5f + posDist(gen) * 0.5f;
        p.color[1] = 0.5f + posDist(gen) * 0.5f;
        p.color[2] = 0.5f + posDist(gen) * 0.5f;
        p.color[3] = 0.8f;
    }
}

void ParticleVisualization::updateParticles(const AudioData& audioData, float deltaTime) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> lifeDist(0.5f, 2.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
    
    // Calculate average amplitude for this frame
    float avgAmplitude = 0.0f;
    for (size_t i = 0; i < audioData.waveformData.size(); i++) {
        avgAmplitude += std::abs(audioData.waveformData[i]);
    }
    avgAmplitude /= audioData.waveformData.size();
    
    // Calculate bass energy (low frequencies)
    float bassEnergy = 0.0f;
    const int bassRange = std::min(8, static_cast<int>(audioData.frequencyData.size() / 4));
    for (int i = 0; i < bassRange; i++) {
        bassEnergy += audioData.frequencyData[i];
    }
    bassEnergy /= bassRange;
    
    // Update particles
    for (auto& p : m_particles) {
        p.life -= deltaTime;
        
        if (p.life <= 0.0f) {
            // Respawn particle
            p.x = posDist(gen);
            p.y = posDist(gen);
            p.vx = posDist(gen) * 0.5f * (1.0f + avgAmplitude * 2.0f);
            p.vy = posDist(gen) * 0.5f * (1.0f + avgAmplitude * 2.0f);
            p.life = lifeDist(gen);
            
            // Make color related to audio
            p.color[0] = 0.4f + bassEnergy * 0.6f;
            p.color[1] = 0.3f + colorDist(gen) * 0.7f;
            p.color[2] = 0.5f + colorDist(gen) * 0.5f;
            p.color[3] = 0.8f;
        } else {
            // Update position
            p.x += p.vx * deltaTime * (1.0f + avgAmplitude * 3.0f);
            p.y += p.vy * deltaTime * (1.0f + avgAmplitude * 3.0f);
            
            // Apply forces based on audio
            if (!audioData.frequencyData.empty()) {
                int freqIndex = std::min(static_cast<int>(audioData.frequencyData.size() - 1),
                                      static_cast<int>((p.x + 1.0f) * 0.5f * audioData.frequencyData.size()));
                
                float force = audioData.frequencyData[freqIndex] * 0.1f;
                p.vx += posDist(gen) * force * deltaTime;
                p.vy += posDist(gen) * force * deltaTime;
            }
            
            // Damping
            p.vx *= 0.99f;
            p.vy *= 0.99f;
            
            // Fade out towards end of life
            if (p.life < 0.3f) {
                p.color[3] = p.life / 0.3f * 0.8f;
            }
        }
    }
}

void ParticleVisualization::render(Renderer* renderer, const AudioData& audioData) {
    int windowWidth, windowHeight;
    renderer->getWindowSize(windowWidth, windowHeight);
    
    // Update particles
    updateParticles(audioData, 1.0f / 60.0f); // Assume 60 fps for now
    
    // Calculate center and scale for rendering
    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    float scale = std::min(windowWidth, windowHeight) * 0.5f;
    
    // Draw particles
    for (const auto& p : m_particles) {
        renderer->setColor(p.color[0], p.color[1], p.color[2], p.color[3]);
        
        float x = centerX + p.x * scale;
        float y = centerY + p.y * scale;
        float size = 5.0f + 10.0f * p.color[3]; // Size based on alpha
        
        renderer->fillCircle(x, y, size);
    }
}

// VisualizationManager implementation
VisualizationManager::VisualizationManager()
    : m_currentVisualizationIndex(0) {
}

VisualizationManager::~VisualizationManager() {
}

void VisualizationManager::initialize() {
    // Create default visualizations
    m_visualizations.push_back(std::make_unique<SpectrumVisualization>());
    m_visualizations.push_back(std::make_unique<WaveformVisualization>());
    m_visualizations.push_back(std::make_unique<CircularVisualization>());
    m_visualizations.push_back(std::make_unique<ParticleVisualization>());
    
    m_currentVisualizationIndex = 0;
}

void VisualizationManager::renderCurrentVisualization(Renderer* renderer, const AudioData& audioData) {
    if (m_visualizations.empty()) {
        return;
    }
    
    auto* currentVis = m_visualizations[m_currentVisualizationIndex].get();
    if (currentVis) {
        currentVis->render(renderer, audioData);
    }
}

void VisualizationManager::nextVisualization() {
    if (m_visualizations.empty()) {
        return;
    }
    
    m_currentVisualizationIndex = (m_currentVisualizationIndex + 1) % m_visualizations.size();
}

void VisualizationManager::previousVisualization() {
    if (m_visualizations.empty()) {
        return;
    }
    
    if (m_currentVisualizationIndex == 0) {
        m_currentVisualizationIndex = m_visualizations.size() - 1;
    } else {
        m_currentVisualizationIndex--;
    }
}

Visualization* VisualizationManager::getCurrentVisualization() {
    if (m_visualizations.empty()) {
        return nullptr;
    }
    
    return m_visualizations[m_currentVisualizationIndex].get();
}

std::string VisualizationManager::getCurrentVisualizationName() const {
    if (m_visualizations.empty()) {
        return "None";
    }
    
    auto* vis = m_visualizations[m_currentVisualizationIndex].get();
    return vis ? vis->getName() : "Unknown";
}

void VisualizationManager::setAmplificationFactor(float factor)
{
    m_amplificationFactor = factor;
    
    // Update the amplification factor for all visualizations
    for (auto& vis : m_visualizations) {
        if (vis) {
            vis->setAmplificationFactor(factor);
        }
    }
    
    std::cout << "Amplification factor set to " << factor << " for all visualizations" << std::endl;
}

float VisualizationManager::getAmplificationFactor() const
{
    if (m_visualizations.empty() || m_currentVisualizationIndex >= m_visualizations.size()) {
        return m_amplificationFactor;
    }
    
    // Return the current visualization's amplification factor
    return m_visualizations[m_currentVisualizationIndex]->getAmplificationFactor();
}

} // namespace av 