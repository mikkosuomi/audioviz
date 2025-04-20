#include "visualizations/NeonMeterVisualizer.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <algorithm> // For std::clamp in C++17
#include <SDL.h>

namespace av {

NeonMeterVisualizer::NeonMeterVisualizer()
    : Visualization("Neon Meters")
    , m_meterWidth(0)
    , m_meterHeight(0)
    , m_meterSpacing(0)
    , m_bassPrev(0.0f)
    , m_midPrev(0.0f)
    , m_treblePrev(0.0f)
    , m_meterX(0)
    , m_meterY(0)
    , m_bassColor(0.2f, 0.6f, 1.0f, 1.0f)       // Blue
    , m_midColor(1.0f, 0.4f, 0.8f, 1.0f)        // Pink
    , m_trebleColor(0.1f, 1.0f, 0.6f, 1.0f)     // Green
    , m_glowColor(1.0f, 1.0f, 1.0f, 0.7f)       // White glow
{
    std::cout << "NeonMeterVisualizer created" << std::endl;
}

NeonMeterVisualizer::~NeonMeterVisualizer()
{
    cleanup();
}

void NeonMeterVisualizer::cleanup()
{
    std::cout << "NeonMeterVisualizer cleaned up" << std::endl;
}

void NeonMeterVisualizer::onResize(int width, int height)
{
    // Calculate meter dimensions based on window size
    m_meterWidth = width * 0.2f;
    m_meterHeight = height * 0.7f;
    m_meterSpacing = width * 0.1f;
    
    // Center the meters horizontally, and position them in the upper half
    m_meterX = (width - (m_meterWidth * 3 + m_meterSpacing * 2)) / 2;
    m_meterY = height * 0.15f;
    
    std::cout << "NeonMeterVisualizer resized to " << width << "x" << height << std::endl;
}

std::string NeonMeterVisualizer::getDescription() const
{
    return "Neon Audio Meters - Bass, Mid, and High frequencies";
}

void NeonMeterVisualizer::render(Renderer* renderer, const AudioData& audioData)
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
    
    // Get raw waveform data if available
    if (audioData.waveform.empty()) {
        // Fallback to processed frequency data if waveform unavailable
        processFrequencyData(audioData);
    } else {
        // Process raw waveform directly - this gives us much better meter behavior
        processWaveformData(audioData);
    }
    
    // Create background
    float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    Color bgColor(0.05f, 0.05f, 0.1f, 1.0f); // Dark blue for neon effect
    renderer->drawFilledRect(0, 0, width, height, bgColor);
    
    // Render each meter
    float x = m_meterX;
    renderMeter(renderer, x, m_meterY, m_meterWidth, m_meterHeight, m_bassPrev, m_bassColor, "BASS");
    
    x += m_meterWidth + m_meterSpacing;
    renderMeter(renderer, x, m_meterY, m_meterWidth, m_meterHeight, m_midPrev, m_midColor, "MID");
    
    x += m_meterWidth + m_meterSpacing;
    renderMeter(renderer, x, m_meterY, m_meterWidth, m_meterHeight, m_treblePrev, m_trebleColor, "HIGH");
    
    // Add some ambient glow effects floating around
    for (int i = 0; i < 3; i++) {
        float glowX = width * 0.5f + sin(time * 0.5f + i * 2.0f) * width * 0.4f;
        float glowY = height * 0.5f + cos(time * 0.7f + i * 1.5f) * height * 0.4f;
        float intensity = 0.3f + 0.2f * sin(time * 2.0f + i);
        
        Color glowColor;
        switch (i % 3) {
            case 0: glowColor = m_bassColor; break;
            case 1: glowColor = m_midColor; break;
            case 2: glowColor = m_trebleColor; break;
        }
        
        renderNeonGlow(renderer, glowX, glowY, 30.0f + 20.0f * sin(time + i), 
                       glowColor, intensity * 0.3f);
    }
}

// New method to directly analyze waveform data for more precise meter readings
void NeonMeterVisualizer::processWaveformData(const AudioData& audioData)
{
    // Split the waveform into frequency ranges by analyzing different parts
    // of the waveform with appropriate filters
    const size_t samples = audioData.waveform.size();
    if (samples == 0) return;
    
    // These will hold our filtered and measured values
    float bassValue = 0.0f;
    float midValue = 0.0f;
    float trebleValue = 0.0f;
    
    // Calculate RMS (root mean square) values for bass, mid, and treble ranges
    // by applying simple filters to the time domain signal
    // This is a simplified approach - a real solution would use FFT
    
    // Bass: use every 8th sample (approximates low frequencies)
    float bassSum = 0.0f;
    int bassCount = 0;
    for (size_t i = 0; i < samples; i += 8) {
        bassSum += audioData.waveform[i] * audioData.waveform[i];
        bassCount++;
    }
    if (bassCount > 0) {
        bassValue = std::sqrt(bassSum / bassCount);
    }
    
    // Mid: use every 4th sample offset by 1 (approximates mid frequencies)
    float midSum = 0.0f;
    int midCount = 0;
    for (size_t i = 1; i < samples; i += 4) {
        midSum += audioData.waveform[i] * audioData.waveform[i];
        midCount++;
    }
    if (midCount > 0) {
        midValue = std::sqrt(midSum / midCount);
    }
    
    // Treble: use every 2nd sample offset by 2 (approximates high frequencies)
    float trebleSum = 0.0f;
    int trebleCount = 0;
    for (size_t i = 2; i < samples; i += 2) {
        trebleSum += audioData.waveform[i] * audioData.waveform[i];
        trebleCount++;
    }
    if (trebleCount > 0) {
        trebleValue = std::sqrt(trebleSum / trebleCount);
    }
    
    // Calibration factor to get values into a proper range
    // This will need adjustment based on the actual audio levels
    const float calibration = 1.5f;  // Reduced from 6.0f to prevent maxing out
    bassValue *= calibration;
    midValue *= calibration;
    trebleValue *= calibration;
    
    // Apply audio compressor-style dynamics:
    // 1. Threshold: level above which we start compressing
    // 2. Ratio: amount of compression above threshold
    // 3. Makeup gain: boost the signal after compression
    const float threshold = 0.1f;    // Increased from 0.05f
    const float ratio = 0.6f;        // Less aggressive compression (1.66:1)
    const float makeupGain = 1.2f;   // Reduced from 2.5f
    
    // Apply compression
    bassValue = compressDynamics(bassValue, threshold, ratio, makeupGain);
    midValue = compressDynamics(midValue, threshold, ratio, makeupGain);
    trebleValue = compressDynamics(trebleValue, threshold, ratio, makeupGain);
    
    // Ensure values are in range [0,1]
    bassValue = std::max(0.0f, std::min(bassValue, 1.0f));
    midValue = std::max(0.0f, std::min(midValue, 1.0f));
    trebleValue = std::max(0.0f, std::min(trebleValue, 1.0f));
    
    // Apply peak detection and smoothing
    updateMeterValue(m_bassPrev, bassValue);
    updateMeterValue(m_midPrev, midValue);
    updateMeterValue(m_treblePrev, trebleValue);
}

// Fallback method that processes pre-calculated frequency data
void NeonMeterVisualizer::processFrequencyData(const AudioData& audioData)
{
    // Use moderate sensitivity values
    const float bassSensitivity = 0.3f;     // Reduced from 0.8f
    const float midSensitivity = 0.4f;      // Reduced from 1.0f
    const float trebleSensitivity = 0.5f;   // Reduced from 1.2f
    
    // Get scaled values
    float bassValue = audioData.bass * bassSensitivity * m_amplificationFactor / 10.0f;
    float midValue = audioData.mid * midSensitivity * m_amplificationFactor / 10.0f;
    float trebleValue = audioData.treble * trebleSensitivity * m_amplificationFactor / 10.0f;
    
    // Apply audio dynamics processing
    const float threshold = 0.1f;    // Match waveform processing
    const float ratio = 0.6f;        // Match waveform processing
    const float makeupGain = 1.2f;   // Match waveform processing
    
    // Apply compression
    bassValue = compressDynamics(bassValue, threshold, ratio, makeupGain);
    midValue = compressDynamics(midValue, threshold, ratio, makeupGain);
    trebleValue = compressDynamics(trebleValue, threshold, ratio, makeupGain);
    
    // Ensure values are in range [0,1]
    bassValue = std::max(0.0f, std::min(bassValue, 1.0f));
    midValue = std::max(0.0f, std::min(midValue, 1.0f));
    trebleValue = std::max(0.0f, std::min(trebleValue, 1.0f));
    
    // Apply peak detection and smoothing
    updateMeterValue(m_bassPrev, bassValue);
    updateMeterValue(m_midPrev, midValue);
    updateMeterValue(m_treblePrev, trebleValue);
}

// Audio compressor function - shapes the dynamic range of the audio signal
float NeonMeterVisualizer::compressDynamics(float input, float threshold, float ratio, float makeupGain)
{
    if (input <= threshold) {
        // Below threshold, no compression
        return input * makeupGain;
    } else {
        // Above threshold, apply compression
        float compressed = threshold + (input - threshold) * ratio;
        return compressed * makeupGain;
    }
}

// Update meter values with peak detection and appropriate attack/release
void NeonMeterVisualizer::updateMeterValue(float& currentValue, float newValue)
{
    // Constants for the meter dynamics
    const float attackTime = 0.001f;  // Fast attack in seconds - determines how quickly meter responds to increases
    const float releaseTime = 0.300f; // Slow release in seconds - determines how quickly meter falls back down
    const float frameTime = 0.016f;   // Assuming 60 fps, each frame is ~16ms
    
    // Calculate coefficient for attack and release
    const float attackCoef = std::exp(-frameTime / attackTime);
    const float releaseCoef = std::exp(-frameTime / releaseTime);
    
    // Choose which coefficient to use based on whether level is rising or falling
    if (newValue > currentValue) {
        // Level is rising - use attack time
        currentValue = attackCoef * currentValue + (1.0f - attackCoef) * newValue;
    } else {
        // Level is falling - use release time
        currentValue = releaseCoef * currentValue + (1.0f - releaseCoef) * newValue;
    }
}

void NeonMeterVisualizer::renderMeter(Renderer* renderer, float x, float y, float width, float height, float value, const Color& color, const std::string& label)
{
    // Define meter properties
    const float borderSize = 4.0f;
    const float innerBorderSize = 2.0f;
    
    // Apply a non-linear transformation to make small movements more visible
    // This is like a "visual expander" - small values get amplified more
    // Uses a milder curve for natural-feeling meter movement
    float displayValue;
    if (value <= 0.0f) {
        displayValue = 0.0f;
    } else {
        // Apply a milder response curve (mix of linear and square root)
        // This makes the meter more responsive to small values but not overly sensitive
        const float linearWeight = 0.7f;  // More linear response
        displayValue = linearWeight * value + (1.0f - linearWeight) * std::sqrt(value);
        
        // Add a slight boost to middle range
        const float midBoost = 0.1f;  // Reduced from 0.2f
        displayValue = displayValue * (1.0f + midBoost * (1.0f - displayValue) * displayValue);
    }
    
    // Draw outer border with glow
    Color glowColor = color;
    glowColor.a = 0.6f;
    renderer->drawRect(x - borderSize, y - borderSize, 
                       width + borderSize * 2, height + borderSize * 2, 
                       glowColor, borderSize);
    
    // Draw inner border
    renderer->drawRect(x, y, width, height, color, innerBorderSize);
    
    // Draw meter background
    Color bgColor(0.1f, 0.1f, 0.15f, 0.9f);
    renderer->drawFilledRect(x + innerBorderSize, y + innerBorderSize, 
                          width - innerBorderSize * 2, height - innerBorderSize * 2, 
                          bgColor);
    
    // Draw meter fill based on displayValue (the visually extended value)
    float fillHeight = (height - innerBorderSize * 2) * displayValue;
    Color fillColor = color;
    fillColor.a = 0.3f;
    renderer->drawFilledRect(x + innerBorderSize, 
                          y + height - innerBorderSize - fillHeight, 
                          width - innerBorderSize * 2, 
                          fillHeight, 
                          fillColor);
    
    // Draw scale markings
    const int numMarks = 10;
    const float markWidth = width * 0.8f;
    const float markHeight = 1.0f;
    
    for (int i = 0; i <= numMarks; i++) {
        float yPos = y + height - (height / numMarks) * i;
        float markWidth = width * 0.3f;
        if (i % 5 == 0) markWidth = width * 0.6f; // Longer marks for major divisions
        
        // Use different colors for different sections of the meter
        Color markColor;
        if (i <= numMarks * 0.6f) {
            // Normal range - use meter color but dimmer
            markColor = color;
            markColor.a = 0.5f;
        } else if (i <= numMarks * 0.8f) {
            // Warning range - yellow-orange
            markColor = Color(1.0f, 0.7f, 0.2f, 0.6f);
        } else {
            // Danger range - red
            markColor = Color(1.0f, 0.3f, 0.3f, 0.7f);
        }
        
        // If current value is at or above this mark, make it brighter
        if (displayValue * numMarks >= i) {
            // Light up marks below the current value
            markColor.a = 0.9f;
            
            // Intensify colors for active marks
            if (i <= numMarks * 0.6f) {
                // Make normal range more vibrant
                markColor.r = std::min(1.0f, color.r * 1.2f);
                markColor.g = std::min(1.0f, color.g * 1.2f);
                markColor.b = std::min(1.0f, color.b * 1.2f);
            } else if (i <= numMarks * 0.8f) {
                // Make warning range more vibrant
                markColor = Color(1.0f, 0.8f, 0.1f, 0.9f);
            } else {
                // Make danger range more vibrant
                markColor = Color(1.0f, 0.1f, 0.1f, 1.0f);
            }
        }
        
        renderer->drawFilledRect(x + (width - markWidth) / 2, 
                              yPos - markHeight / 2, 
                              markWidth, 
                              markHeight, 
                              markColor);
    }
    
    // Draw label
    float labelY = y + height + borderSize * 3;
    Color labelColor = color;
    labelColor.a = 0.9f;
    
    // We'll use a simple shape to represent text since drawText is not implemented
    // Draw a color bar for the label
    float labelBarHeight = 15.0f;
    renderer->drawFilledRect(x, labelY, width, labelBarHeight, labelColor);
    
    // Add some glow to the label
    for (float i = 1; i < 5; i++) {
        Color glowColor = labelColor;
        glowColor.a = 0.1f * (5.0f - i);
        renderer->drawRect(x - i, labelY - i, width + i * 2, labelBarHeight + i * 2, glowColor, 1.0f);
    }
    
    // Draw needle
    const float needleWidth = 3.0f;  // Increased from 2.0f
    float needleY = y + height - (height * displayValue);  // Use displayValue instead of value
    
    // Needle base
    renderer->drawFilledRect(x + width * 0.2f,  // Start from 20% of width
                          needleY - needleWidth / 2, 
                          width * 0.6f,  // Extend to 80% of width
                          needleWidth, 
                          color);
    
    // Add a stronger glow around the needle
    for (float i = 0.5f; i <= 2.0f; i += 0.5f) {
        Color glowColor = color;
        glowColor.a = 0.3f * (2.0f - i * 0.4f);
        renderer->drawFilledRect(x + width * 0.2f,
                               needleY - needleWidth/2 - i,
                               width * 0.6f,
                               needleWidth + i * 2.0f,
                               glowColor);
    }
    
    // Enhanced needle pivot (circle at the end)
    float pivotRadius = width * 0.06f;  // Scale with meter width
    renderNeonGlow(renderer, x + width * 0.2f, needleY, pivotRadius, color, 1.0f);
}

void NeonMeterVisualizer::renderNeonGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity)
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

void NeonMeterVisualizer::setAmplificationFactor(float factor)
{
    m_amplificationFactor = factor;
}

float NeonMeterVisualizer::getAmplificationFactor() const
{
    return m_amplificationFactor;
}

} // namespace av 