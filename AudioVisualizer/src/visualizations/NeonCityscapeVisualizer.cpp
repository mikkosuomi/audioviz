#include "visualizations/NeonCityscapeVisualizer.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <SDL.h>

namespace av {

NeonCityscapeVisualizer::NeonCityscapeVisualizer()
    : Visualization("Neon Cityscape")
    , m_width(0)
    , m_height(0)
    , m_horizon(0)
    , m_time(0.0f)
    , m_bassResponse(0.0f)
    , m_midResponse(0.0f)
    , m_trebleResponse(0.0f)
    , m_beatIntensity(0.0f)
    , m_beatActive(false)
    , m_skyTopColor(0.05f, 0.05f, 0.15f, 1.0f)      // Dark blue
    , m_skyBottomColor(0.15f, 0.0f, 0.3f, 1.0f)     // Deep purple
    , m_groundColor(0.0f, 0.0f, 0.0f, 1.0f)         // Black ground
{
    // Initialize random number generator
    std::random_device rd;
    m_rng = std::mt19937(rd());
    
    std::cout << "NeonCityscapeVisualizer created" << std::endl;
}

NeonCityscapeVisualizer::~NeonCityscapeVisualizer()
{
    cleanup();
}

void NeonCityscapeVisualizer::cleanup()
{
    m_buildings.clear();
    m_raindrops.clear();
    m_vehicles.clear();
    std::cout << "NeonCityscapeVisualizer cleaned up" << std::endl;
}

void NeonCityscapeVisualizer::onResize(int width, int height)
{
    m_width = width;
    m_height = height;
    
    // Set horizon line at 70% of screen height
    m_horizon = height * 0.7f;
    
    // Initialize scene elements
    initBuildings();
    initRain();
    initVehicles();
    
    std::cout << "NeonCityscapeVisualizer resized to " << width << "x" << height << std::endl;
}

std::string NeonCityscapeVisualizer::getDescription() const
{
    return "Neon Cityscape - Cyberpunk skyline that pulses with the music";
}

void NeonCityscapeVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    // Check if rendering surface size changed
    if (m_width != renderer->getWidth() || m_height != renderer->getHeight()) {
        onResize(renderer->getWidth(), renderer->getHeight());
    }
    
    // Update time
    float deltaTime = 1.0f / 60.0f; // Assuming 60 fps
    m_time += deltaTime;
    
    // Process audio data
    processAudio(audioData);
    
    // Draw sky gradient
    for (int y = 0; y < m_horizon; y++) {
        float t = y / m_horizon;
        Color skyColor;
        skyColor.r = m_skyTopColor.r * (1.0f - t) + m_skyBottomColor.r * t;
        skyColor.g = m_skyTopColor.g * (1.0f - t) + m_skyBottomColor.g * t;
        skyColor.b = m_skyTopColor.b * (1.0f - t) + m_skyBottomColor.b * t;
        skyColor.a = 1.0f;
        
        // Add subtle audio-reactive tint to sky
        float bassEffect = m_bassResponse * 0.1f;
        float trebleEffect = m_trebleResponse * 0.05f;
        skyColor.r += bassEffect;
        skyColor.b += trebleEffect;
        
        renderer->drawLine(0, y, m_width, y, skyColor, 1.0f);
    }
    
    // Draw ground
    renderer->drawFilledRect(0, m_horizon, m_width, m_height - m_horizon, m_groundColor);
    
    // Draw grid lines on ground (perspective)
    const int gridLines = 10;
    for (int i = 0; i <= gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        float y = m_horizon + (m_height - m_horizon) * t;
        
        // Horizontal lines get dimmer with distance
        float alpha = 0.3f * (1.0f - t);
        Color gridColor(0.0f, 1.0f, 1.0f, alpha * (0.5f + m_midResponse * 0.5f));
        
        // Horizontal grid line
        renderer->drawLine(0, y, m_width, y, gridColor, 1.0f);
        
        // Vertical grid lines with perspective
        for (int j = 0; j <= 20; j++) {
            float x = m_width * (j / 20.0f);
            float perspectiveY1 = m_horizon;
            float perspectiveY2 = m_height;
            
            // Adjust for perspective converging to center
            x = (x - m_width/2) * (1.0f + t) + m_width/2;
            
            if (x >= 0 && x <= m_width) {
                renderer->drawLine(x, perspectiveY1, x, perspectiveY2, gridColor, 1.0f);
            }
        }
    }
    
    // Render scene elements
    renderRain(renderer);
    renderSkyline(renderer);
    renderVehicles(renderer);
    
    // Update rain
    for (auto& drop : m_raindrops) {
        drop.y += drop.speed * deltaTime * (1.0f + m_bassResponse);
        if (drop.y > m_height) {
            // Reset raindrop at top with random x position
            std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(m_width));
            drop.x = xDist(m_rng);
            drop.y = -drop.length;
        }
    }
    
    // Update vehicles
    for (auto& vehicle : m_vehicles) {
        if (vehicle.rightToLeft) {
            vehicle.x -= vehicle.speed * deltaTime * (1.0f + m_midResponse * 0.5f);
            if (vehicle.x < -vehicle.size * 2) {
                // Reset at right side
                vehicle.x = m_width + vehicle.size;
                std::uniform_real_distribution<float> yDist(m_horizon * 0.2f, m_horizon * 0.6f);
                vehicle.y = yDist(m_rng);
            }
        } else {
            vehicle.x += vehicle.speed * deltaTime * (1.0f + m_midResponse * 0.5f);
            if (vehicle.x > m_width + vehicle.size * 2) {
                // Reset at left side
                vehicle.x = -vehicle.size;
                std::uniform_real_distribution<float> yDist(m_horizon * 0.2f, m_horizon * 0.6f);
                vehicle.y = yDist(m_rng);
            }
        }
    }
    
    // Update buildings based on audio
    for (auto& building : m_buildings) {
        // Animate building height
        float targetHeight = building.height;
        
        // Buildings respond differently based on their x position and audio bands
        float xPos = building.x / m_width; // 0 to 1 across the screen
        
        if (xPos < 0.33f) {
            // Left side buildings react to bass
            targetHeight *= (0.7f + m_bassResponse * 0.5f);
            building.pulse = m_bassResponse;
        } else if (xPos < 0.66f) {
            // Middle buildings react to mids
            targetHeight *= (0.7f + m_midResponse * 0.5f);
            building.pulse = m_midResponse;
        } else {
            // Right side buildings react to treble
            targetHeight *= (0.7f + m_trebleResponse * 0.5f);
            building.pulse = m_trebleResponse;
        }
        
        // Add beat response
        if (m_beatActive) {
            targetHeight *= (1.0f + m_beatIntensity * 0.2f);
            building.pulse += m_beatIntensity * 0.5f;
        }
        
        // Smoothly animate to target height
        building.currentHeight = building.currentHeight * 0.9f + targetHeight * 0.1f;
    }
}

void NeonCityscapeVisualizer::initBuildings()
{
    // Clear existing buildings
    m_buildings.clear();
    
    // Distribute buildings across screen
    const int buildingCount = 30;
    const float minWidth = m_width * 0.02f;  // Minimum building width
    const float maxWidth = m_width * 0.05f;  // Maximum building width
    const float minHeight = m_height * 0.1f; // Minimum building height
    const float maxHeight = m_height * 0.4f; // Maximum building height
    
    std::uniform_real_distribution<float> widthDist(minWidth, maxWidth);
    std::uniform_real_distribution<float> heightDist(minHeight, maxHeight);
    std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);
    std::uniform_int_distribution<int> windowsDist(3, 8);
    
    float x = 0.0f;
    
    for (int i = 0; i < buildingCount; i++) {
        Building building;
        building.width = widthDist(m_rng);
        building.height = heightDist(m_rng);
        building.currentHeight = building.height;
        building.x = x;
        building.color = randomNeonColor();
        building.speed = speedDist(m_rng);
        building.windows = windowsDist(m_rng);
        building.pulse = 0.0f;
        
        m_buildings.push_back(building);
        
        // Add some spacing between buildings
        x += building.width * 1.2f;
    }
    
    // Sort buildings by height for proper drawing order
    std::sort(m_buildings.begin(), m_buildings.end(), 
              [](const Building& a, const Building& b) {
                  return a.height > b.height;
              });
}

void NeonCityscapeVisualizer::initRain()
{
    // Clear existing raindrops
    m_raindrops.clear();
    
    // Create rain particles
    const int rainCount = 300;
    
    std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(m_width));
    std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(m_height));
    std::uniform_real_distribution<float> speedDist(300.0f, 600.0f);
    std::uniform_real_distribution<float> lengthDist(10.0f, 25.0f);
    std::uniform_real_distribution<float> alphaDist(0.1f, 0.5f);
    
    for (int i = 0; i < rainCount; i++) {
        RainDrop drop;
        drop.x = xDist(m_rng);
        drop.y = yDist(m_rng);
        drop.speed = speedDist(m_rng);
        drop.length = lengthDist(m_rng);
        drop.alpha = alphaDist(m_rng);
        
        m_raindrops.push_back(drop);
    }
}

void NeonCityscapeVisualizer::initVehicles()
{
    // Clear existing vehicles
    m_vehicles.clear();
    
    // Create flying vehicles
    const int vehicleCount = 10;
    
    std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(m_width));
    std::uniform_real_distribution<float> yDist(m_horizon * 0.2f, m_horizon * 0.6f);
    std::uniform_real_distribution<float> speedDist(50.0f, 150.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 15.0f);
    std::uniform_int_distribution<int> dirDist(0, 1);
    
    for (int i = 0; i < vehicleCount; i++) {
        Vehicle vehicle;
        vehicle.x = xDist(m_rng);
        vehicle.y = yDist(m_rng);
        vehicle.speed = speedDist(m_rng);
        vehicle.size = sizeDist(m_rng);
        vehicle.color = randomNeonColor();
        vehicle.rightToLeft = dirDist(m_rng) == 1;
        
        m_vehicles.push_back(vehicle);
    }
}

Color NeonCityscapeVisualizer::randomNeonColor()
{
    // Generate vibrant neon colors
    const Color neonColors[] = {
        Color(1.0f, 0.3f, 0.9f, 1.0f),  // Pink
        Color(0.2f, 1.0f, 0.9f, 1.0f),  // Cyan
        Color(0.9f, 0.9f, 0.2f, 1.0f),  // Yellow
        Color(0.9f, 0.2f, 0.2f, 1.0f),  // Red
        Color(0.2f, 0.9f, 0.2f, 1.0f),  // Green
        Color(0.6f, 0.2f, 0.9f, 1.0f),  // Purple
        Color(0.9f, 0.5f, 0.1f, 1.0f)   // Orange
    };
    
    std::uniform_int_distribution<int> colorDist(0, 6);
    return neonColors[colorDist(m_rng)];
}

void NeonCityscapeVisualizer::renderSkyline(Renderer* renderer)
{
    // Draw each building
    for (const auto& building : m_buildings) {
        renderBuilding(renderer, building);
    }
}

void NeonCityscapeVisualizer::renderBuilding(Renderer* renderer, const Building& building)
{
    // Building base
    Color buildingColor = building.color;
    
    // Darker base color
    Color baseColor(buildingColor.r * 0.3f, buildingColor.g * 0.3f, buildingColor.b * 0.3f, 1.0f);
    
    float buildingTop = m_horizon - building.currentHeight;
    
    // Draw building silhouette
    renderer->drawFilledRect(building.x, buildingTop, building.width, building.currentHeight, baseColor);
    
    // Draw windows
    float windowWidth = building.width / (building.windows + 1);
    float windowHeight = building.currentHeight / 10.0f;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < building.windows; col++) {
            float windowX = building.x + (col + 1) * (building.width / (building.windows + 1)) - windowWidth / 2;
            float windowY = buildingTop + row * windowHeight + windowHeight * 0.25f;
            
            // Randomize window illumination
            float seed = static_cast<float>(row * 100 + col) + m_time * 2.0f;
            float noise = (std::sin(seed) + 1.0f) * 0.5f; // 0 to 1
            
            // Brighter with audio
            float brightness = 0.5f + noise * 0.5f + building.pulse * 0.5f;
            Color windowColor(
                buildingColor.r * brightness,
                buildingColor.g * brightness,
                buildingColor.b * brightness,
                0.9f
            );
            
            // Draw window
            renderer->drawFilledRect(
                windowX, 
                windowY, 
                windowWidth * 0.7f, 
                windowHeight * 0.5f,
                windowColor
            );
        }
    }
    
    // Draw building top edge with glow
    float edgeThickness = 2.0f;
    Color edgeColor = building.color;
    edgeColor.a = 0.8f + building.pulse * 0.2f;
    renderer->drawRect(
        building.x, 
        buildingTop, 
        building.width, 
        building.currentHeight,
        edgeColor,
        edgeThickness
    );
    
    // Draw neon sign on top
    float signWidth = building.width * 0.7f;
    float signHeight = building.width * 0.3f;
    float signX = building.x + (building.width - signWidth) * 0.5f;
    float signY = buildingTop - signHeight * 0.8f;
    
    // Only draw signs on taller buildings
    if (building.height > m_height * 0.25f) {
        Color signColor = building.color;
        signColor.a = 0.9f + building.pulse * 0.1f;
        
        // Draw sign background
        Color signBgColor(0.0f, 0.0f, 0.0f, 0.8f);
        renderer->drawFilledRect(signX, signY, signWidth, signHeight, signBgColor);
        
        // Draw sign border
        renderer->drawRect(signX, signY, signWidth, signHeight, signColor, 2.0f);
        
        // Draw sign glow
        renderGlow(renderer, signX + signWidth/2, signY + signHeight/2, signWidth/2, signColor, 0.3f + building.pulse * 0.3f);
    }
}

void NeonCityscapeVisualizer::renderRain(Renderer* renderer)
{
    // Draw rain streaks
    Color rainColor(0.8f, 0.9f, 1.0f, 0.4f);
    
    for (const auto& drop : m_raindrops) {
        // Adjust alpha based on bass response for visual effect
        Color dropColor = rainColor;
        dropColor.a = drop.alpha * (1.0f + m_bassResponse * 0.2f);
        
        // Draw a line for the raindrop
        renderer->drawLine(
            drop.x, 
            drop.y, 
            drop.x, 
            drop.y - drop.length,
            dropColor,
            1.0f
        );
    }
}

void NeonCityscapeVisualizer::renderVehicles(Renderer* renderer)
{
    for (const auto& vehicle : m_vehicles) {
        // Draw vehicle body
        Color vehicleColor = vehicle.color;
        
        // Adjust color based on audio
        vehicleColor.a = 0.8f + m_trebleResponse * 0.2f;
        
        // Draw main light
        float lightSize = vehicle.size * 0.8f;
        renderGlow(renderer, vehicle.x, vehicle.y, lightSize, vehicleColor, 0.7f);
        
        // Draw smaller trail lights
        int trailCount = 3;
        for (int i = 1; i <= trailCount; i++) {
            float trailX = vehicle.rightToLeft ? 
                vehicle.x + i * vehicle.size * 0.7f : 
                vehicle.x - i * vehicle.size * 0.7f;
                
            float alpha = 0.4f * (1.0f - static_cast<float>(i) / trailCount);
            Color trailColor = vehicleColor;
            trailColor.a = alpha;
            
            renderGlow(renderer, trailX, vehicle.y, lightSize * 0.6f, trailColor, 0.4f);
        }
    }
}

void NeonCityscapeVisualizer::renderGlow(Renderer* renderer, float x, float y, float radius, const Color& color, float intensity)
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

void NeonCityscapeVisualizer::processAudio(const AudioData& audioData)
{
    // Apply smooth transitions to audio responses
    const float smoothFactor = 0.1f;
    
    m_bassResponse = m_bassResponse * (1.0f - smoothFactor) + audioData.bass * smoothFactor;
    m_midResponse = m_midResponse * (1.0f - smoothFactor) + audioData.mid * smoothFactor;
    m_trebleResponse = m_trebleResponse * (1.0f - smoothFactor) + audioData.treble * smoothFactor;
    
    // Apply amplification
    m_bassResponse *= (m_amplificationFactor / 20.0f);
    m_midResponse *= (m_amplificationFactor / 20.0f);
    m_trebleResponse *= (m_amplificationFactor / 20.0f);
    
    // Detect beats for pulsing effects
    float newBeatIntensity = getBeatIntensity(audioData);
    
    // Apply beat detection with some hysteresis
    const float beatThreshold = 0.2f;
    if (newBeatIntensity > beatThreshold && newBeatIntensity > m_beatIntensity * 1.2f) {
        m_beatActive = true;
        m_beatIntensity = newBeatIntensity;
    } else {
        m_beatIntensity = m_beatIntensity * 0.9f + newBeatIntensity * 0.1f;
        if (m_beatIntensity < beatThreshold * 0.5f) {
            m_beatActive = false;
        }
    }
}

float NeonCityscapeVisualizer::getBeatIntensity(const AudioData& audioData)
{
    // Simple beat detection focusing on bass and sudden amplitude changes
    float bassEnergy = audioData.bass * 1.2f;
    float midEnergy = audioData.mid * 0.8f;
    
    return bassEnergy * 0.7f + midEnergy * 0.3f;
}

void NeonCityscapeVisualizer::setAmplificationFactor(float factor)
{
    m_amplificationFactor = factor;
}

float NeonCityscapeVisualizer::getAmplificationFactor() const
{
    return m_amplificationFactor;
}

} // namespace av 