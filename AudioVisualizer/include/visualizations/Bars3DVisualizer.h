#pragma once

#include "../Visualization.h"
#include <vector>

namespace av {

class Bars3DVisualizer : public Visualization {
public:
    Bars3DVisualizer();
    virtual ~Bars3DVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    virtual bool initialize(Renderer* renderer) override;
    void cleanup();
    void onResize(int width, int height);

private:
    struct Bar {
        float height;      // Current height
        float targetHeight; // Target height based on audio
        float x, z;        // Position in 3D space
        float hue;         // Color
    };

    void updateBars(const AudioData& audioData, float deltaTime);
    void setup3DView(int width, int height);
    void draw3DBars(Renderer* renderer);
    void reset3DView();
    
    std::vector<Bar> m_bars;
    int m_gridSize;        // Number of bars in each direction
    float m_maxBarHeight;
    float m_barWidth;
    float m_spacing;
    float m_rotationAngle;
    float m_cameraHeight;
    float m_lastUpdateTime;
    float m_smoothingFactor;
};

} // namespace av 