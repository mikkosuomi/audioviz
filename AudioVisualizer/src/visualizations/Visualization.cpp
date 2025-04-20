#include "Visualization.h"
#include <iostream>
#include <cmath>

namespace av {

// --------------------------------------------------------
// Visualization class implementation
// --------------------------------------------------------

Visualization::Visualization(const std::string& name)
    : m_name(name)
    , m_initialized(false)
{
}

Visualization::~Visualization()
{
    if (m_initialized) {
        cleanup();
    }
}

bool Visualization::initialize(Renderer* renderer)
{
    m_initialized = true;
    return true;
}

void Visualization::update(const AudioData& audioData, float deltaTime)
{
    // Default implementation does nothing
}

void Visualization::cleanup()
{
    m_initialized = false;
}

const std::string& Visualization::getName() const
{
    return m_name;
}

void Visualization::onResize(int width, int height)
{
    // Default implementation does nothing
}

// --------------------------------------------------------
// VisualizationManager class implementation
// --------------------------------------------------------

VisualizationManager::VisualizationManager()
    : m_currentVisualizationIndex(0)
{
}

VisualizationManager::~VisualizationManager()
{
    // Will automatically delete all visualizations through unique_ptr
}

void VisualizationManager::initialize()
{
    // Add default visualizations
    m_visualizations.push_back(std::make_unique<SpectrumVisualization>());
    m_visualizations.push_back(std::make_unique<WaveformVisualization>());
    m_visualizations.push_back(std::make_unique<CircularVisualization>());
    m_visualizations.push_back(std::make_unique<ParticleVisualization>());
    
    m_currentVisualizationIndex = 0;
}

void VisualizationManager::renderCurrentVisualization(Renderer* renderer, const AudioData& audioData)
{
    if (m_visualizations.empty()) {
        return;
    }
    
    auto* viz = getCurrentVisualization();
    if (viz) {
        viz->render(renderer, audioData);
    }
}

void VisualizationManager::nextVisualization()
{
    if (m_visualizations.empty()) {
        return;
    }
    
    m_currentVisualizationIndex = (m_currentVisualizationIndex + 1) % m_visualizations.size();
}

void VisualizationManager::previousVisualization()
{
    if (m_visualizations.empty()) {
        return;
    }
    
    if (m_currentVisualizationIndex == 0) {
        m_currentVisualizationIndex = m_visualizations.size() - 1;
    } else {
        m_currentVisualizationIndex--;
    }
}

Visualization* VisualizationManager::getCurrentVisualization()
{
    if (m_visualizations.empty()) {
        return nullptr;
    }
    
    return m_visualizations[m_currentVisualizationIndex].get();
}

std::string VisualizationManager::getCurrentVisualizationName() const
{
    if (m_visualizations.empty()) {
        return "None";
    }
    
    return m_visualizations[m_currentVisualizationIndex]->getName();
}

// --------------------------------------------------------
// Visualization subclass implementations
// --------------------------------------------------------

void SpectrumVisualization::render(Renderer* renderer, const AudioData& audioData)
{
    // Draw spectrum visualization here
    int width = 800;
    int height = 600;
    
    float barWidth = width / static_cast<float>(audioData.spectrum.size());
    
    for (size_t i = 0; i < audioData.spectrum.size(); i++) {
        float barHeight = audioData.spectrum[i] * height * 0.8f;
        float x = i * barWidth;
        float y = height - barHeight;
        
        Color color = Color::fromHSV(static_cast<float>(i) / audioData.spectrum.size() * 0.8f, 0.8f, 1.0f);
        renderer->drawFilledRect(x, y, barWidth - 1, barHeight, color);
    }
}

void WaveformVisualization::render(Renderer* renderer, const AudioData& audioData)
{
    // Draw waveform visualization here
    int width = 800;
    int height = 600;
    int centerY = height / 2;
    
    if (audioData.waveform.empty()) {
        return;
    }
    
    float xScale = width / static_cast<float>(audioData.waveform.size());
    float yScale = height * 0.4f;
    
    for (size_t i = 1; i < audioData.waveform.size(); i++) {
        float x1 = (i - 1) * xScale;
        float y1 = centerY + audioData.waveform[i - 1] * yScale;
        float x2 = i * xScale;
        float y2 = centerY + audioData.waveform[i] * yScale;
        
        Color color(0.0f, 1.0f, 0.0f, 1.0f);
        renderer->drawLine(x1, y1, x2, y2, color);
    }
}

void CircularVisualization::render(Renderer* renderer, const AudioData& audioData)
{
    // Draw circular visualization here
    int width = 800;
    int height = 600;
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float radius = std::min(width, height) * 0.4f;
    
    for (size_t i = 0; i < audioData.spectrum.size(); i++) {
        float angle = static_cast<float>(i) / audioData.spectrum.size() * 6.28f;
        float amplitude = audioData.spectrum[i] * radius;
        float innerRadius = radius * 0.5f;
        float outerRadius = innerRadius + amplitude;
        
        float x1 = centerX + innerRadius * cos(angle);
        float y1 = centerY + innerRadius * sin(angle);
        float x2 = centerX + outerRadius * cos(angle);
        float y2 = centerY + outerRadius * sin(angle);
        
        Color color = Color::fromHSV(static_cast<float>(i) / audioData.spectrum.size() * 0.8f, 0.8f, 1.0f);
        renderer->drawLine(x1, y1, x2, y2, color, 2.0f);
    }
}

void ParticleVisualization::render(Renderer* renderer, const AudioData& audioData)
{
    // Draw particle visualization here
    int width = 800;
    int height = 600;
    
    for (int i = 0; i < 20; i++) {
        float energy = audioData.bass * 0.5f + audioData.mid * 0.3f + audioData.treble * 0.2f;
        float size = 10.0f + energy * 50.0f;
        float x = width * (0.2f + 0.6f * (i % 5) / 4.0f);
        float y = height * (0.2f + 0.6f * (i / 5) / 3.0f);
        
        Color color = Color::fromHSV(i / 20.0f + energy * 0.2f, 0.8f, 0.8f + energy * 0.2f);
        renderer->drawFilledCircle(x, y, size, color);
    }
}

} // namespace av 