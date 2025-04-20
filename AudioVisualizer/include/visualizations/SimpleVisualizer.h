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

private:
    int m_barCount;
    float m_barWidth;
    float m_barSpacing;
    float m_maxBarHeight;
    Color m_backgroundColor;
};

} // namespace av 