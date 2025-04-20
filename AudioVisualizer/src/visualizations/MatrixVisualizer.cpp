#include "visualizations/MatrixVisualizer.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <random>
#include <cmath>
#include <SDL.h>
#include <GL/glew.h>

namespace av {

MatrixVisualizer::MatrixVisualizer()
    : Visualization("Matrix")
    , m_columnCount(80)
    , m_symbolSize(16.0f)
    , m_lastUpdateTime(0.0f)
{
    std::cout << "MatrixVisualizer created" << std::endl;
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(10.0f, 50.0f);
    std::uniform_real_distribution<float> posDist(0.0f, 1000.0f);
    std::uniform_real_distribution<float> hueDist(0.3f, 0.4f); // Green hues
    
    // Initialize columns with random values
    m_columns.resize(m_columnCount);
    for (auto& column : m_columns) {
        column.speed = speedDist(gen);
        column.position = posDist(gen);
        column.hue = hueDist(gen);
        
        // Initialize with random ASCII characters
        int symbolCount = static_cast<int>(std::round(1000.0f / column.speed));
        column.symbols.resize(symbolCount);
        for (auto& symbol : column.symbols) {
            // Use characters that look like code/matrix symbols
            symbol = static_cast<char>(gen() % 94 + 33); // Printable ASCII
        }
    }
}

MatrixVisualizer::~MatrixVisualizer()
{
    cleanup();
}

void MatrixVisualizer::cleanup()
{
    // Nothing to clean up
}

void MatrixVisualizer::onResize(int width, int height)
{
    // Adjust number of columns based on width
    m_columnCount = width / static_cast<int>(m_symbolSize);
    
    // Recreate columns
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(10.0f, 50.0f);
    std::uniform_real_distribution<float> posDist(0.0f, static_cast<float>(height));
    std::uniform_real_distribution<float> hueDist(0.3f, 0.4f); // Green hues
    
    m_columns.resize(m_columnCount);
    for (auto& column : m_columns) {
        column.speed = speedDist(gen);
        column.position = posDist(gen);
        column.hue = hueDist(gen);
        
        // Initialize with random ASCII characters
        int symbolCount = static_cast<int>(std::round(height / m_symbolSize));
        column.symbols.resize(symbolCount);
        for (auto& symbol : column.symbols) {
            symbol = static_cast<char>(gen() % 94 + 33); // Printable ASCII
        }
    }
}

void MatrixVisualizer::updateColumns(const AudioData& audioData, float deltaTime)
{
    // Get energy levels to affect the visualization
    float bassInfluence = audioData.bass * 3.0f;  // Bass affects speed
    float midInfluence = audioData.mid * 2.0f;    // Mid affects color
    float trebleInfluence = audioData.treble * 2.0f; // Treble affects brightness
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> charDist(33, 126); // Printable ASCII
    
    // Update each column
    for (auto& column : m_columns) {
        // Update position based on speed and bass influence
        column.position += column.speed * (1.0f + bassInfluence) * deltaTime;
        
        // Change hue based on mid-frequency content
        column.hue = 0.3f + midInfluence * 0.1f; // Shift between green hues
        
        // Randomly change characters based on treble
        if (trebleInfluence > 0.5f) {
            for (auto& symbol : column.symbols) {
                if (gen() % 10 == 0) { // 10% chance to change
                    symbol = static_cast<char>(charDist(gen));
                }
            }
        }
    }
}

void MatrixVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // Calculate delta time for animation
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - m_lastUpdateTime) / 1000.0f;
    m_lastUpdateTime = currentTime;
    
    // Update falling code columns
    updateColumns(audioData, deltaTime);
    
    // Draw black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Calculate column width
    float columnWidth = static_cast<float>(width) / m_columnCount;
    
    // Draw each column
    for (int i = 0; i < m_columnCount; i++) {
        auto& column = m_columns[i];
        float x = i * columnWidth;
        
        // Loop the column position if it's gone too far
        while (column.position > height + column.symbols.size() * m_symbolSize) {
            column.position -= height + column.symbols.size() * m_symbolSize;
        }
        
        // Draw each symbol in the column
        for (size_t j = 0; j < column.symbols.size(); j++) {
            float y = column.position - j * m_symbolSize;
            
            // Skip if outside screen
            if (y < -m_symbolSize || y > height) {
                continue;
            }
            
            // Fade out based on position (brightest at the head of the column)
            float fade = 1.0f - static_cast<float>(j) / column.symbols.size();
            fade = std::pow(fade, 2.0f); // Make the fade-out more pronounced
            
            // Boost brightness based on audio energy
            fade = std::min(1.0f, fade + audioData.energy * 0.3f);
            
            // Set color based on hue and position
            float saturation = 0.8f;
            float value = fade * 0.8f + 0.2f; // Never completely black
            
            Color color = Color::fromHSV(column.hue, saturation, value);
            
            // Draw the symbol (simplified as a rectangle with varying brightness)
            renderer->drawFilledRect(x, y, columnWidth, m_symbolSize, color);
            
            // Draw the brightest character at the head
            if (j == 0) {
                Color headColor = Color::fromHSV(column.hue, 0.5f, 1.0f);
                renderer->drawFilledRect(x, y, columnWidth, m_symbolSize, headColor);
            }
        }
    }
}

std::string MatrixVisualizer::getDescription() const
{
    return "Digital rain effect inspired by The Matrix";
}

} // namespace av 