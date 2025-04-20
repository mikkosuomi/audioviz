#include "visualizations/SimpleVisualizer.h"
#include <algorithm>
#include <iostream>
#include <SDL.h>
#include <GL/glew.h>  // Add GLEW header for OpenGL functions
#include <cmath>

namespace av {

SimpleVisualizer::SimpleVisualizer()
    : Visualization("Simple")
    , m_barCount(64)
    , m_barWidth(8.0f)
    , m_barSpacing(2.0f)
    , m_maxBarHeight(300.0f)
    , m_backgroundColor(1.0f, 0.5f, 0.0f, 1.0f) // BRIGHT ORANGE for visibility
    , m_amplificationFactor(15.0f)
{
    std::cout << "SimpleVisualizer created - Using BRIGHT ORANGE BACKGROUND for visibility" << std::endl;
}

SimpleVisualizer::~SimpleVisualizer()
{
    cleanup();
}

void SimpleVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    int centerX = width / 2;
    int centerY = height / 2;
    
    // Track frame count for animations
    static int frameCount = 0;
    frameCount++;
    
    // Create flashy background that pulses with the audio energy
    float energyPulse = 0.3f + audioData.energy * 0.7f; // Increased visibility
    
    // Create a gradient background that reacts to the audio
    Color topColor = Color::fromHSV(frameCount * 0.01f, 0.9f, energyPulse);
    Color bottomColor = Color::fromHSV(frameCount * 0.01f + 0.5f, 0.9f, energyPulse * 0.7f + 0.3f);
    
    // Draw gradient background
    glBegin(GL_QUADS);
    glColor4f(topColor.r, topColor.g, topColor.b, topColor.a);
    glVertex2f(0, 0);                      // Top left
    glVertex2f(width, 0);                  // Top right
    glColor4f(bottomColor.r, bottomColor.g, bottomColor.b, bottomColor.a);
    glVertex2f(width, height);             // Bottom right
    glVertex2f(0, height);                 // Bottom left
    glEnd();
    
    // Apply amplification to make visualization more responsive
    float amplifiedBass = std::min(1.0f, audioData.bass * m_amplificationFactor);
    float amplifiedMid = std::min(1.0f, audioData.mid * m_amplificationFactor);
    float amplifiedTreble = std::min(1.0f, audioData.treble * m_amplificationFactor);
    float amplifiedEnergy = std::min(1.0f, audioData.energy * m_amplificationFactor);

    // Render waveform visualization in upper part of the screen
    if (!audioData.waveform.empty()) {
        // Debug info - print dimensions
        std::cout << "Drawing waveform area: width=" << width << ", height=" << height 
                 << ", samples=" << audioData.waveform.size() << std::endl;
        
        // Create a visible background for the waveform - use a consistent bright color
        Color bgColor = Color(0.0f, 0.0f, 0.2f, 1.0f); // Dark blue background
        float waveHeight = height / 2.5f; // Make it larger
        float waveTop = 30.0f;
        float waveBottom = waveTop + waveHeight;
        float waveMid = waveTop + waveHeight / 2.0f;
        
        // Draw waveform background and border
        renderer->drawFilledRect(20, waveTop, width - 40, waveHeight, bgColor);
        
        // Draw a pulse effect around the waveform with the bass
        float pulseSize = 10.0f + amplifiedBass * 20.0f;
        Color pulseColor = Color(1.0f, 1.0f, 0.0f, 1.0f); // Bright yellow
        renderer->drawRect(20, waveTop, width - 40, waveHeight, pulseColor, pulseSize);
        
        // Draw center line
        renderer->drawLine(20, waveMid, width - 20, waveMid, 
                         Color(1.0f, 1.0f, 1.0f, 0.8f), 3.0f);
        
        // Use a bright color for the waveform
        Color waveformColor = Color(1.0f, 0.0f, 0.0f, 1.0f); // Bright red
        
        // Very thick lines for visibility
        const float lineThickness = 8.0f; // Increased from 5.0f
        
        // Debug output
        std::cout << "Rendering waveform with " << audioData.waveform.size() 
                 << " samples, amplification: " << m_amplificationFactor << std::endl;
        
        // Calculate average amplitude for debugging
        float totalAmplitude = 0.0f;
        float maxAmplitude = 0.0f;
        for (size_t i = 0; i < audioData.waveform.size(); ++i) {
            totalAmplitude += std::abs(audioData.waveform[i]);
            maxAmplitude = std::max(maxAmplitude, std::abs(audioData.waveform[i]));
        }
        float averageAmplitude = audioData.waveform.size() > 0 ? 
                               totalAmplitude / audioData.waveform.size() : 0.0f;
        
        std::cout << "Waveform diagnostics - Avg amplitude: " << averageAmplitude 
                 << ", Max amplitude: " << maxAmplitude 
                 << ", After amplification: " << maxAmplitude * m_amplificationFactor << std::endl;
        
        // Draw waveform lines with increased amplitude
        float prevX = 20;
        float prevY = waveMid;
        const float pointSpacing = static_cast<float>(width - 40) / audioData.waveform.size();
        
        for (size_t i = 0; i < audioData.waveform.size(); ++i) {
            float sample = audioData.waveform[i] * m_amplificationFactor; // Apply amplification
            sample = std::clamp(sample, -1.0f, 1.0f); // Clamp to prevent extreme values
            
            // If the signal is too small, add a minimum visibility
            if (std::abs(sample) < 0.05f) {
                sample = sample >= 0 ? 0.05f : -0.05f;
            }
            
            float currentX = 20 + i * pointSpacing;
            float currentY = waveMid + (sample * waveHeight / 2.0f);
            
            // Draw line segment
            renderer->drawLine(prevX, prevY, currentX, currentY, waveformColor, lineThickness);
            
            prevX = currentX;
            prevY = currentY;
        }
    }

    // Draw spectrum - enhanced version with 3D effect
    if (!audioData.spectrum.empty()) {
        // Make the spectrum bigger and position it below the waveform
        int spectrumY = height / 2;
        int spectrumHeight = height / 3;
        
        int barCount = std::min(128, static_cast<int>(audioData.spectrum.size()));
        float barWidth = static_cast<float>(width - 40) / barCount;
        
        // Draw a background for the spectrum
        Color spectrumBgColor = Color::fromHSV(frameCount * 0.01f + 0.3f, 0.3f, 0.2f);
        renderer->drawFilledRect(20, spectrumY, width - 40, spectrumHeight, spectrumBgColor);
        
        // Draw border that pulses with treble
        float borderThickness = 2.0f + amplifiedTreble * 8.0f;
        Color borderColor = Color::fromHSV(frameCount * 0.01f + 0.3f, 0.9f, 0.7f);
        renderer->drawRect(20, spectrumY, width - 40, spectrumHeight, borderColor, borderThickness);

        // Draw spectrum bars with 3D effect
        for (int i = 0; i < barCount; i++) {
            float index = static_cast<float>(i) / barCount * std::min(static_cast<int>(audioData.spectrum.size() - 1), 512);
            int specIndex = static_cast<int>(index);
            
            // Apply an envelope to emphasize the shape
            float amplifiedValue = std::min(1.0f, audioData.spectrum[specIndex] * m_amplificationFactor);
            
            // Add some bounce based on the bass
            float bounceEffect = 0.0f;
            if (i < barCount / 3) {
                // Low frequencies respond to bass
                bounceEffect = amplifiedBass * 0.3f * std::sin(frameCount * 0.1f + i * 0.05f);
            } else if (i < barCount * 2 / 3) {
                // Mid frequencies respond to mids
                bounceEffect = amplifiedMid * 0.2f * std::sin(frameCount * 0.15f + i * 0.05f);
            } else {
                // High frequencies respond to treble
                bounceEffect = amplifiedTreble * 0.1f * std::sin(frameCount * 0.2f + i * 0.05f);
            }
            
            float finalHeight = (amplifiedValue + bounceEffect) * spectrumHeight;
            
            // Position of the bar
            float x = 20 + i * barWidth;
            float y = spectrumY + spectrumHeight - finalHeight;
            
            // Color based on frequency and amplitude
            float hue = static_cast<float>(i) / barCount * 0.6f; // Use 60% of the hue spectrum
            float saturation = 0.8f + amplifiedValue * 0.2f;     // More intense colors for louder frequencies
            float brightness = 0.7f + amplifiedValue * 0.3f;     // Brighter for louder
            
            Color barTopColor = Color::fromHSV(hue, saturation, brightness);
            Color barBottomColor = Color::fromHSV(hue, saturation * 0.8f, brightness * 0.5f);
            
            // Draw 3D bar with gradient
            glBegin(GL_QUADS);
            // Top of bar
            glColor4f(barTopColor.r, barTopColor.g, barTopColor.b, barTopColor.a);
            glVertex2f(x, y);
            glVertex2f(x + barWidth - 1, y);
            
            // Bottom of bar
            glColor4f(barBottomColor.r, barBottomColor.g, barBottomColor.b, barBottomColor.a);
            glVertex2f(x + barWidth - 1, spectrumY + spectrumHeight);
            glVertex2f(x, spectrumY + spectrumHeight);
            glEnd();
            
            // Add highlight to top of bar
            renderer->drawLine(x, y, x + barWidth - 1, y, 
                             Color(1.0f, 1.0f, 1.0f, 0.7f), 2.0f);
        }
    }

    // Draw circular audio indicators at the bottom
    int circleY = height - height / 6;
    
    // Bass circle - Red
    float bassRadius = 40.0f + amplifiedBass * 60.0f;
    float bassPulse = 1.0f + amplifiedEnergy * std::sin(frameCount * 0.1f) * 0.3f;
    bassRadius *= bassPulse;
    renderer->drawFilledCircle(width / 4, circleY, bassRadius, 
                             Color::fromHSV(0.0f, 0.9f, 0.9f, 0.8f));
    
    // Mid circle - Green
    float midRadius = 30.0f + amplifiedMid * 50.0f;
    float midPulse = 1.0f + amplifiedEnergy * std::sin(frameCount * 0.12f + 0.4f) * 0.3f;
    midRadius *= midPulse;
    renderer->drawFilledCircle(width / 2, circleY, midRadius, 
                             Color::fromHSV(0.33f, 0.9f, 0.9f, 0.8f));
    
    // Treble circle - Blue
    float trebleRadius = 20.0f + amplifiedTreble * 40.0f;
    float treblePulse = 1.0f + amplifiedEnergy * std::sin(frameCount * 0.14f + 0.8f) * 0.3f;
    trebleRadius *= treblePulse;
    renderer->drawFilledCircle(width * 3 / 4, circleY, trebleRadius, 
                             Color::fromHSV(0.66f, 0.9f, 0.9f, 0.8f));
    
    // Draw energy bar at the very bottom
    int energyBarHeight = 20;
    int energyBarWidth = static_cast<int>(width * amplifiedEnergy);
    
    // Energy bar background
    renderer->drawFilledRect(0, height - energyBarHeight, width, energyBarHeight,
                           Color(0.1f, 0.1f, 0.1f, 0.7f));
    
    // Energy bar with gradient based on energy level
    Color energyStartColor = Color::fromHSV(0.0f, 0.9f, 0.9f); // Red at low energy
    Color energyEndColor = Color::fromHSV(0.3f, 0.9f, 0.9f);   // Green at high energy
    
    // Interpolate between colors based on energy
    float t = amplifiedEnergy;
    Color energyColor(
        energyStartColor.r * (1-t) + energyEndColor.r * t,
        energyStartColor.g * (1-t) + energyEndColor.g * t,
        energyStartColor.b * (1-t) + energyEndColor.b * t,
        1.0f
    );
    
    renderer->drawFilledRect(0, height - energyBarHeight, energyBarWidth, energyBarHeight, energyColor);
}

void SimpleVisualizer::cleanup()
{
    // Nothing to clean up in this simple implementation
}

void SimpleVisualizer::onResize(int width, int height)
{
    // Update visualization parameters based on new window size
    m_barCount = std::min(64, width / 10);
    m_barWidth = std::max(4.0f, static_cast<float>(width) / static_cast<float>(m_barCount) / 2.0f);
    m_barSpacing = m_barWidth / 4.0f;
    m_maxBarHeight = height * 0.75f;
    
    std::cout << "SimpleVisualizer resized to: " << width << "x" << height << std::endl;
}

std::string SimpleVisualizer::getDescription() const
{
    return "Simple bar-based spectrum visualizer";
}

} // namespace av