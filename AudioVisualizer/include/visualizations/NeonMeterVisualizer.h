#pragma once

#include "../Visualization.h"
#include <vector>

namespace av {

class NeonMeterVisualizer : public Visualization {
public:
    NeonMeterVisualizer();
    virtual ~NeonMeterVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);
    
    // Set amplification factor for the meters
    void setAmplificationFactor(float factor);
    float getAmplificationFactor() const;

private:
    // Audio processing methods
    void processWaveformData(const AudioData& audioData);
    void processFrequencyData(const AudioData& audioData);
    float compressDynamics(float input, float threshold, float ratio, float makeupGain);
    void updateMeterValue(float& currentValue, float newValue);
    
    // Design parameters
    float m_meterWidth;
    float m_meterHeight;
    float m_meterSpacing;
    
    // Store previous values for smooth transitions
    float m_bassPrev;
    float m_midPrev;
    float m_treblePrev;
    
    // Track meter positions and sizes
    float m_meterX;
    float m_meterY;
    
    // Colors for the neon effect
    Color m_bassColor;
    Color m_midColor;
    Color m_trebleColor;
    Color m_glowColor;
    
    // Amplification factor 
    float m_amplificationFactor = 20.0f;
    
    // Helper methods for rendering parts of the visualization
    void renderMeter(Renderer* renderer, float x, float y, float width, float height, 
                     float value, const Color& color, const std::string& label);
    void renderNeonGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity);
};

} // namespace av 