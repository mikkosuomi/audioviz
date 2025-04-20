#pragma once

#include "../Visualization.h"
#include <vector>

namespace av {

class MatrixVisualizer : public Visualization {
public:
    MatrixVisualizer();
    virtual ~MatrixVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);

private:
    struct Column {
        std::vector<char> symbols;
        float speed;
        float position;
        float hue;
    };

    void updateColumns(const AudioData& audioData, float deltaTime);
    
    std::vector<Column> m_columns;
    int m_columnCount;
    float m_symbolSize;
    float m_lastUpdateTime;
};

} // namespace av 