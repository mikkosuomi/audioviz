#include "visualizations/RetroWaveOscilloscopeVisualizer.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <SDL.h>
#include <random>

namespace av {

RetroWaveOscilloscopeVisualizer::RetroWaveOscilloscopeVisualizer()
    : Visualization("RetroWave Oscilloscope")
    , m_width(0)
    , m_height(0)
    , m_horizon(0)
    , m_gridSpacingX(0)
    , m_gridSpacingY(0)
    , m_waveformHeight(0)
    , m_waveformWidth(0)
    , m_waveformY(0)
    , m_time(0)
    , m_bassResponse(0)
    , m_midResponse(0)
    , m_trebleResponse(0)
    , m_skyTopColor(0.05f, 0.0f, 0.2f, 1.0f)           // Deep purple
    , m_skyBottomColor(0.8f, 0.2f, 0.5f, 1.0f)         // Pink/magenta
    , m_gridColor(0.0f, 0.8f, 0.8f, 0.6f)              // Cyan
    , m_waveformColor(1.0f, 0.4f, 0.8f, 1.0f)          // Hot pink
    , m_horizonColor(0.9f, 0.4f, 0.7f, 1.0f)           // Bright pink
    , m_rng(std::random_device{}())
{
    std::cout << "RetroWaveOscilloscopeVisualizer created" << std::endl;
}

RetroWaveOscilloscopeVisualizer::~RetroWaveOscilloscopeVisualizer()
{
    cleanup();
}

void RetroWaveOscilloscopeVisualizer::cleanup()
{
    std::cout << "RetroWaveOscilloscopeVisualizer cleaned up" << std::endl;
}

void RetroWaveOscilloscopeVisualizer::onResize(int width, int height)
{
    m_width = width;
    m_height = height;
    
    // Set horizon line at 60% of the screen height
    m_horizon = m_height * 0.60f;
    
    // Set grid spacing
    m_gridSpacingX = m_width / 20.0f;
    m_gridSpacingY = m_height / 20.0f;
    
    // Set waveform properties
    m_waveformWidth = m_width * 0.9f;
    m_waveformHeight = m_height * 0.3f;
    m_waveformY = m_horizon - m_waveformHeight * 0.5f;
    
    // Initialize elements
    initGrid();
    initMountains();
    initSun();
    initStars();
    
    std::cout << "RetroWaveOscilloscopeVisualizer resized to " << width << "x" << height << std::endl;
}

std::string RetroWaveOscilloscopeVisualizer::getDescription() const
{
    return "RetroWave Oscilloscope - 80s style waveform visualization";
}

void RetroWaveOscilloscopeVisualizer::initGrid()
{
    // Nothing to store - grid is procedurally rendered
}

void RetroWaveOscilloscopeVisualizer::initMountains()
{
    m_mountains.clear();
    
    // Create several mountain ranges with different heights and colors
    std::uniform_real_distribution<float> widthDist(m_width * 0.1f, m_width * 0.4f);
    std::uniform_real_distribution<float> heightDist(m_height * 0.05f, m_height * 0.2f);
    
    // First range (farthest)
    int mountainCount = 5;
    float x = 0;
    
    // Add purple mountains in the back
    while (x < m_width) {
        Mountain mountain;
        mountain.x = x;
        mountain.width = widthDist(m_rng);
        mountain.height = heightDist(m_rng) * 0.6f;
        mountain.color = Color(0.4f, 0.1f, 0.4f, 0.8f); // Purple
        m_mountains.push_back(mountain);
        x += mountain.width * 0.7f; // Overlap mountains
    }
    
    // Second range (closer)
    x = -m_width * 0.1f;
    while (x < m_width) {
        Mountain mountain;
        mountain.x = x;
        mountain.width = widthDist(m_rng);
        mountain.height = heightDist(m_rng);
        mountain.color = Color(0.6f, 0.1f, 0.5f, 0.9f); // More vibrant purple
        m_mountains.push_back(mountain);
        x += mountain.width * 0.7f;
    }
}

void RetroWaveOscilloscopeVisualizer::initSun()
{
    m_sun.x = m_width * 0.5f;
    m_sun.y = m_horizon - m_height * 0.05f;
    m_sun.radius = m_height * 0.15f;
    m_sun.glow = 1.0f;
    m_sun.color = Color(1.0f, 0.7f, 0.3f, 1.0f); // Orange-yellow
}

void RetroWaveOscilloscopeVisualizer::initStars()
{
    m_stars.clear();
    
    // Create 100 random stars
    std::uniform_real_distribution<float> xDist(0, m_width);
    std::uniform_real_distribution<float> yDist(0, m_horizon - m_height * 0.1f);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> brightnessDist(0.3f, 1.0f);
    std::uniform_real_distribution<float> pulseDist(0.5f, 2.0f);
    
    for (int i = 0; i < 100; i++) {
        Star star;
        star.x = xDist(m_rng);
        star.y = yDist(m_rng);
        star.size = sizeDist(m_rng);
        star.brightness = brightnessDist(m_rng);
        star.pulse = pulseDist(m_rng);
        m_stars.push_back(star);
    }
}

void RetroWaveOscilloscopeVisualizer::processAudio(const AudioData& audioData)
{
    // Apply amplification factor
    const float sensitivity = 0.5f;
    const float bassSensitivity = 0.6f;
    const float midSensitivity = 0.5f;
    const float trebleSensitivity = 0.3f;
    
    // Get scaled values
    float bassValue = audioData.bass * bassSensitivity * m_amplificationFactor / 10.0f;
    float midValue = audioData.mid * midSensitivity * m_amplificationFactor / 10.0f;
    float trebleValue = audioData.treble * trebleSensitivity * m_amplificationFactor / 10.0f;
    
    // Apply smoothing
    const float smoothingFactor = 0.2f;
    m_bassResponse = m_bassResponse * (1.0f - smoothingFactor) + bassValue * smoothingFactor;
    m_midResponse = m_midResponse * (1.0f - smoothingFactor) + midValue * smoothingFactor;
    m_trebleResponse = m_trebleResponse * (1.0f - smoothingFactor) + trebleValue * smoothingFactor;
    
    // Ensure values are in range [0,1]
    m_bassResponse = std::max(0.0f, std::min(m_bassResponse, 1.0f));
    m_midResponse = std::max(0.0f, std::min(m_midResponse, 1.0f));
    m_trebleResponse = std::max(0.0f, std::min(m_trebleResponse, 1.0f));
}

void RetroWaveOscilloscopeVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // If first render or window size changed, recalculate sizes
    static int lastWidth = 0;
    static int lastHeight = 0;
    if (width != lastWidth || height != lastHeight) {
        onResize(width, height);
        lastWidth = width;
        lastHeight = height;
    }
    
    // Update time and process audio
    m_time += 0.016f; // Assuming 60fps
    processAudio(audioData);
    
    // Render layers from back to front
    renderBackground(renderer);
    renderStars(renderer);
    renderSun(renderer);
    renderMountains(renderer);
    renderHorizon(renderer);
    renderGrid(renderer);
    renderWaveform(renderer, audioData);
}

void RetroWaveOscilloscopeVisualizer::renderBackground(Renderer* renderer)
{
    // Draw color gradient background
    for (int y = 0; y < m_height; y++) {
        // Calculate gradient color
        float t = static_cast<float>(y) / m_horizon;
        t = std::min(1.0f, t);
        
        Color color(
            m_skyTopColor.r + (m_skyBottomColor.r - m_skyTopColor.r) * t,
            m_skyTopColor.g + (m_skyBottomColor.g - m_skyTopColor.g) * t,
            m_skyTopColor.b + (m_skyBottomColor.b - m_skyTopColor.b) * t,
            1.0f
        );
        
        renderer->drawLine(0, y, m_width, y, color);
    }
    
    // Fill the rest below horizon with black
    Color groundColor(0.0f, 0.0f, 0.0f, 1.0f);
    renderer->drawFilledRect(0, m_horizon, m_width, m_height - m_horizon, groundColor);
}

void RetroWaveOscilloscopeVisualizer::renderGrid(Renderer* renderer)
{
    // Draw horizontal grid lines
    for (float y = m_horizon; y < m_height; y += m_gridSpacingY) {
        // Calculate alpha based on distance from horizon
        float distFactor = (y - m_horizon) / (m_height - m_horizon);
        float alpha = 1.0f - distFactor * 0.8f;
        
        Color lineColor = m_gridColor;
        lineColor.a = alpha * (0.4f + m_midResponse * 0.6f);
        
        renderer->drawLine(0, y, m_width, y, lineColor);
    }
    
    // Draw vertical grid lines
    for (float x = 0; x < m_width; x += m_gridSpacingX) {
        // Calculate vanishing point perspective scaling
        float perspectiveScale = 2.0f;
        float horizonMidX = m_width / 2.0f;
        float perspectiveFactor = std::abs(x - horizonMidX) / horizonMidX;
        float spacingY = m_gridSpacingY * (1.0f + perspectiveScale * perspectiveFactor);
        
        for (float y = m_horizon + spacingY; y < m_height; y += spacingY) {
            // Perspective alpha
            float yDistFactor = (y - m_horizon) / (m_height - m_horizon);
            float alpha = 1.0f - yDistFactor * 0.8f;
            
            Color lineColor = m_gridColor;
            lineColor.a = alpha * (0.4f + m_midResponse * 0.6f);
            
            // Draw shorter line at this y position
            float xDist = std::abs(x - horizonMidX);
            float startX = horizonMidX + (x - horizonMidX) * (1.0f - yDistFactor * 0.5f);
            float endX = startX + (x < horizonMidX ? -1.0f : 1.0f) * 
                         m_gridSpacingX * 0.5f * (1.0f - yDistFactor * 0.5f);
            
            renderer->drawLine(startX, y, endX, y, lineColor);
        }
    }
}

void RetroWaveOscilloscopeVisualizer::renderHorizon(Renderer* renderer)
{
    // Draw horizon line
    Color horizonColor = m_horizonColor;
    horizonColor.a = 0.8f + m_midResponse * 0.2f;
    renderer->drawLine(0, m_horizon, m_width, m_horizon, horizonColor, 3.0f);
    
    // Add glow to horizon line
    for (float i = 1.0f; i <= 5.0f; i += 1.0f) {
        Color glowColor = horizonColor;
        glowColor.a = (0.6f - i * 0.1f) * (0.5f + m_bassResponse * 0.5f);
        renderer->drawLine(0, m_horizon + i, m_width, m_horizon + i, glowColor);
        renderer->drawLine(0, m_horizon - i, m_width, m_horizon - i, glowColor);
    }
}

void RetroWaveOscilloscopeVisualizer::renderMountains(Renderer* renderer)
{
    for (const auto& mountain : m_mountains) {
        // Generate mountain points
        std::vector<float> points;
        
        // Start at the base
        points.push_back(mountain.x);
        points.push_back(m_horizon);
        
        // Generate jagged mountain shape
        const int steps = 20;
        float peakX = mountain.x + mountain.width * 0.5f;
        float peakY = m_horizon - mountain.height;
        
        for (int i = 0; i <= steps; i++) {
            float x = mountain.x + (mountain.width * i) / steps;
            
            // Distance from peak (0.0 = at peak, 1.0 = at edge)
            float distFromPeak = std::abs(x - peakX) / (mountain.width * 0.5f);
            distFromPeak = std::min(1.0f, distFromPeak);
            
            // Basic mountain curve
            float baseHeight = mountain.height * (1.0f - distFromPeak * distFromPeak);
            
            // Add some randomness for jagged peaks
            std::uniform_real_distribution<float> jitter(-mountain.height * 0.1f, mountain.height * 0.05f);
            float jitterAmount = (i > 0 && i < steps) ? jitter(m_rng) : 0.0f;
            
            float y = m_horizon - baseHeight + jitterAmount;
            
            points.push_back(x);
            points.push_back(y);
        }
        
        // Close the polygon at the base
        points.push_back(mountain.x + mountain.width);
        points.push_back(m_horizon);
        
        // Draw the mountain
        renderer->drawFilledPolygon(points.data(), points.size() / 2, mountain.color);
        
        // Add a slight top highlight
        Color highlightColor = mountain.color;
        highlightColor.r += 0.1f;
        highlightColor.g += 0.1f;
        highlightColor.b += 0.1f;
        
        for (size_t i = 2; i < points.size() - 2; i += 2) {
            renderer->drawLine(points[i], points[i+1], points[i+2], points[i+3], highlightColor, 2.0f);
        }
    }
}

void RetroWaveOscilloscopeVisualizer::renderSun(Renderer* renderer)
{
    // Sun position can be affected by bass
    float sunY = m_sun.y - m_bassResponse * m_height * 0.05f;
    
    // Draw the sun
    Color sunColor = m_sun.color;
    float sunPulse = 0.8f + 0.2f * std::sin(m_time * 0.5f);
    float radius = m_sun.radius * (0.9f + 0.1f * sunPulse + 0.1f * m_bassResponse);
    
    // Draw sun disc
    renderer->drawFilledCircle(m_sun.x, sunY, radius, sunColor);
    
    // Draw sun glow
    renderGlow(renderer, m_sun.x, sunY, radius * 1.2f, sunColor, 0.7f * (0.8f + 0.2f * m_bassResponse));
    renderGlow(renderer, m_sun.x, sunY, radius * 1.8f, sunColor, 0.4f * (0.8f + 0.2f * m_bassResponse));
    renderGlow(renderer, m_sun.x, sunY, radius * 2.5f, sunColor, 0.2f * (0.8f + 0.2f * m_bassResponse));
    
    // Draw grid lines over the sun (vertical)
    const int sunLines = 8;
    for (int i = 0; i < sunLines; i++) {
        float x = m_sun.x - radius + (2.0f * radius * i) / (sunLines - 1);
        float alpha = 0.3f * (0.5f + 0.5f * m_trebleResponse);
        Color lineColor(0.0f, 0.0f, 0.0f, alpha);
        
        renderer->drawLine(x, sunY - radius, x, sunY + radius, lineColor);
    }
    
    // Draw grid lines over the sun (horizontal)
    for (int i = 0; i < sunLines; i++) {
        float y = sunY - radius + (2.0f * radius * i) / (sunLines - 1);
        float alpha = 0.3f * (0.5f + 0.5f * m_trebleResponse);
        Color lineColor(0.0f, 0.0f, 0.0f, alpha);
        
        renderer->drawLine(m_sun.x - radius, y, m_sun.x + radius, y, lineColor);
    }
}

void RetroWaveOscilloscopeVisualizer::renderStars(Renderer* renderer)
{
    for (auto& star : m_stars) {
        // Make stars twinkle
        float time = m_time * star.pulse;
        float brightness = star.brightness * (0.7f + 0.3f * std::sin(time));
        brightness += m_trebleResponse * 0.2f; // Make stars respond to high frequencies
        
        // Draw star
        Color starColor(1.0f, 1.0f, 1.0f, brightness);
        renderer->drawFilledCircle(star.x, star.y, star.size, starColor);
        
        // Add a simple glow
        Color glowColor(1.0f, 1.0f, 1.0f, brightness * 0.5f);
        renderer->drawCircle(star.x, star.y, star.size * 2.0f, glowColor);
    }
}

void RetroWaveOscilloscopeVisualizer::renderWaveform(Renderer* renderer, const AudioData& audioData)
{
    if (audioData.waveform.empty()) return;
    
    // Waveform dimensions
    float width = m_waveformWidth;
    float height = m_waveformHeight * (0.8f + 0.4f * m_midResponse);
    float x = (m_width - width) / 2.0f;
    float y = m_waveformY;
    
    // Apply amplification to the waveform
    std::vector<float> amplifiedWaveform;
    amplifiedWaveform.reserve(audioData.waveform.size());
    
    float amplification = m_amplificationFactor * 1.0f;
    for (float sample : audioData.waveform) {
        amplifiedWaveform.push_back(sample * amplification);
    }
    
    // Ensure values are clamped
    for (auto& sample : amplifiedWaveform) {
        sample = std::max(-1.0f, std::min(1.0f, sample));
    }
    
    // Create points for the waveform line
    std::vector<float> points;
    const size_t sampleCount = amplifiedWaveform.size();
    const size_t step = std::max(size_t(1), sampleCount / 200); // Limit to ~200 points
    
    for (size_t i = 0; i < sampleCount; i += step) {
        float xPos = x + (width * i) / sampleCount;
        float yPos = y + (amplifiedWaveform[i] * height * 0.5f);
        points.push_back(xPos);
        points.push_back(yPos);
    }
    
    if (points.size() < 4) return; // Need at least 2 points
    
    // Draw the waveform with glow effect
    Color waveColor = m_waveformColor;
    
    // Make color react to audio
    waveColor.r = 0.8f + 0.2f * m_midResponse;
    waveColor.g = 0.3f + 0.2f * m_bassResponse;
    waveColor.b = 0.7f + 0.3f * m_trebleResponse;
    
    // Draw main waveform line
    const float lineThickness = 2.0f;
    for (size_t i = 0; i < points.size() - 2; i += 2) {
        renderer->drawLine(points[i], points[i+1], points[i+2], points[i+3], waveColor, lineThickness);
    }
    
    // Add glow to the waveform
    Color glowColor = waveColor;
    glowColor.a = 0.5f;
    
    for (float thickness = 2.0f; thickness <= 6.0f; thickness += 2.0f) {
        glowColor.a = 0.5f / thickness;
        for (size_t i = 0; i < points.size() - 2; i += 2) {
            renderer->drawLine(points[i], points[i+1], points[i+2], points[i+3], glowColor, thickness);
        }
    }
    
    // Draw reflection of waveform in the grid
    Color reflectionColor = waveColor;
    reflectionColor.a = 0.3f;
    
    for (size_t i = 0; i < points.size() - 2; i += 2) {
        float reflectionY1 = m_horizon + (m_horizon - points[i+1]) * 0.2f;
        float reflectionY2 = m_horizon + (m_horizon - points[i+3]) * 0.2f;
        
        // Make the reflection fade with distance
        float dist1 = (reflectionY1 - m_horizon) / (m_height - m_horizon);
        float dist2 = (reflectionY2 - m_horizon) / (m_height - m_horizon);
        
        Color color1 = reflectionColor;
        Color color2 = reflectionColor;
        color1.a *= (1.0f - dist1 * 0.8f);
        color2.a *= (1.0f - dist2 * 0.8f);
        
        renderer->drawLine(points[i], reflectionY1, points[i+2], reflectionY2, reflectionColor, 1.0f);
    }
}

void RetroWaveOscilloscopeVisualizer::renderGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity)
{
    // Draw concentric circles with decreasing alpha to create glow effect
    const int numLayers = 5;
    for (int i = 0; i < numLayers; i++) {
        float r = radius * (1.0f + i * 0.5f);
        Color glowColor = color;
        glowColor.a = intensity * (1.0f - (float)i / numLayers);
        
        renderer->drawCircle(x, y, r, glowColor);
    }
}

void RetroWaveOscilloscopeVisualizer::setAmplificationFactor(float factor)
{
    m_amplificationFactor = factor;
    std::cout << "RetroWaveOscilloscopeVisualizer amplification set to " << factor << std::endl;
}

float RetroWaveOscilloscopeVisualizer::getAmplificationFactor() const
{
    return m_amplificationFactor;
}

} // namespace av 