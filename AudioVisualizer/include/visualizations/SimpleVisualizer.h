#pragma once

#include "../Visualization.h"

namespace av {

class SimpleVisualizer : public Visualization {
public:
    SimpleVisualizer();
    virtual ~SimpleVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);
    
    // Add method to set amplification factor
    void setAmplificationFactor(float factor) { m_amplificationFactor = factor; }
    float getAmplificationFactor() const { return m_amplificationFactor; }

private:
    int m_barCount;
    float m_barWidth;
    float m_barSpacing;
    float m_maxBarHeight;
    Color m_backgroundColor;
    float m_amplificationFactor = 20.0f; // Default value
};

} // namespace av 