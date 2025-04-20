#pragma once

#include "../Visualization.h"
#include <vector>
#include <random>

namespace av {

/**
 * @brief RetroWave style oscilloscope visualization with neon grid and waveform display
 */
class RetroWaveOscilloscopeVisualizer : public Visualization {
public:
    RetroWaveOscilloscopeVisualizer();
    virtual ~RetroWaveOscilloscopeVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);
    
    // Set amplification factor for the visualization
    void setAmplificationFactor(float factor);
    float getAmplificationFactor() const;

private:
    // Sun structure
    struct Sun {
        float x, y;          // Position
        float radius;        // Size
        float glow;          // Glow intensity
        Color color;         // Sun color
    };
    
    // Mountain profile
    struct Mountain {
        float x;             // Start position
        float width;         // Width
        float height;        // Height
        Color color;         // Color
    };
    
    // Star in the sky
    struct Star {
        float x, y;          // Position
        float size;          // Size
        float brightness;    // Brightness
        float pulse;         // Pulse rate
    };
    
    // Initialize elements
    void initGrid();
    void initMountains();
    void initSun();
    void initStars();
    
    // Render elements
    void renderBackground(Renderer* renderer);
    void renderGrid(Renderer* renderer);
    void renderHorizon(Renderer* renderer);
    void renderMountains(Renderer* renderer);
    void renderSun(Renderer* renderer);
    void renderStars(Renderer* renderer);
    void renderWaveform(Renderer* renderer, const AudioData& audioData);
    void renderGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity);
    
    // Audio processing
    void processAudio(const AudioData& audioData);
    
    // Dimensions
    int m_width;
    int m_height;
    float m_horizon;       // Horizon line height
    
    // Grid properties
    float m_gridSpacingX;
    float m_gridSpacingY;
    
    // Visual elements
    Sun m_sun;
    std::vector<Mountain> m_mountains;
    std::vector<Star> m_stars;
    
    // Waveform properties
    float m_waveformHeight;
    float m_waveformWidth;
    float m_waveformY;
    
    // Color palette
    Color m_skyTopColor;
    Color m_skyBottomColor;
    Color m_gridColor;
    Color m_waveformColor;
    Color m_horizonColor;
    
    // Animation state
    float m_time;
    float m_bassResponse;
    float m_midResponse;
    float m_trebleResponse;
    
    // Random number generator
    std::mt19937 m_rng;
    
    // Amplification factor
    float m_amplificationFactor = 20.0f;
};

} // namespace av 