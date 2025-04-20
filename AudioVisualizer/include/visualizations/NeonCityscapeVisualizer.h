#pragma once

#include "../Visualization.h"
#include <vector>
#include <random>

namespace av {

/**
 * @brief Cyberpunk-style city skyline visualization with neon buildings that pulse to the music
 */
class NeonCityscapeVisualizer : public Visualization {
public:
    NeonCityscapeVisualizer();
    virtual ~NeonCityscapeVisualizer();

    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    virtual std::string getDescription() const override;
    void cleanup();
    void onResize(int width, int height);
    
    // Set amplification factor for the visualization
    void setAmplificationFactor(float factor);
    float getAmplificationFactor() const;

private:
    // Building structure
    struct Building {
        float x;           // Building position
        float width;       // Building width
        float height;      // Building max height
        float currentHeight; // Current animated height
        Color color;       // Building color
        float speed;       // Animation speed
        int windows;       // Number of windows
        float pulse;       // Pulse effect (0-1)
    };
    
    // Rain particle
    struct RainDrop {
        float x, y;        // Position
        float speed;       // Fall speed
        float length;      // Streak length
        float alpha;       // Transparency
    };
    
    // Flying vehicle
    struct Vehicle {
        float x, y;        // Position
        float speed;       // Movement speed
        float size;        // Vehicle size
        Color color;       // Vehicle color
        bool rightToLeft;  // Direction
    };
    
    // Initialize cityscape
    void initBuildings();
    void initRain();
    void initVehicles();
    
    // Generate a random neon color
    Color randomNeonColor();
    
    // Render elements
    void renderSkyline(Renderer* renderer);
    void renderBuilding(Renderer* renderer, const Building& building);
    void renderRain(Renderer* renderer);
    void renderVehicles(Renderer* renderer);
    void renderGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity);
    
    // Audio processing
    void processAudio(const AudioData& audioData);
    float getBeatIntensity(const AudioData& audioData);
    
    // Dimensions
    int m_width;
    int m_height;
    float m_horizon;       // Horizon line height
    
    // Building array
    std::vector<Building> m_buildings;
    
    // Rain array
    std::vector<RainDrop> m_raindrops;
    
    // Vehicles
    std::vector<Vehicle> m_vehicles;
    
    // Background gradients
    Color m_skyTopColor;
    Color m_skyBottomColor;
    Color m_groundColor;
    
    // Animation states
    float m_time;           // Running time
    float m_bassResponse;   // Low frequency response
    float m_midResponse;    // Mid frequency response
    float m_trebleResponse; // High frequency response
    float m_beatIntensity;  // Beat detection intensity
    bool m_beatActive;      // Beat detection state
    
    // Random number generator
    std::mt19937 m_rng;
    
    // Amplification factor
    float m_amplificationFactor = 20.0f;
};

} // namespace av 