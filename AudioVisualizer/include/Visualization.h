#pragma once

#include "AudioProcessor.h"
#include "Renderer.h"
#include <string>
#include <memory>
#include <vector>

namespace av {

/**
 * Base class for all visualizations
 */
class Visualization {
public:
    Visualization(const std::string& name);
    virtual ~Visualization();
    
    // Initialization and lifecycle methods
    virtual bool initialize(Renderer* renderer);
    virtual void update(const AudioData& audioData, float deltaTime);
    virtual void cleanup();
    virtual void onResize(int width, int height);
    
    // Render the visualization
    virtual void render(Renderer* renderer, const AudioData& audioData) = 0;
    
    // Get the name of this visualization
    const std::string& getName() const;
    
    // Get visualization description
    virtual std::string getDescription() const { return ""; }

protected:
    std::string m_name;
    bool m_initialized;
};

/**
 * Visualization that displays spectrum data as bars
 */
class SpectrumVisualization : public Visualization {
public:
    SpectrumVisualization() : Visualization("Spectrum") {}
    
    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    
    std::string getDescription() const override { 
        return "Classic spectrum analyzer with bars"; 
    }
};

/**
 * Visualization that displays the audio waveform
 */
class WaveformVisualization : public Visualization {
public:
    WaveformVisualization() : Visualization("Waveform") {}
    
    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    
    std::string getDescription() const override { 
        return "Oscilloscope-style waveform display"; 
    }
};

/**
 * Visualization that displays a circular spectrum
 */
class CircularVisualization : public Visualization {
public:
    CircularVisualization() : Visualization("Circular") {}
    
    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    
    std::string getDescription() const override { 
        return "Circular frequency spectrum with reactive rings"; 
    }
};

/**
 * Visualization that uses particles driven by audio data
 */
class ParticleVisualization : public Visualization {
public:
    ParticleVisualization() : Visualization("Particle") {}
    
    virtual void render(Renderer* renderer, const AudioData& audioData) override;
    
    std::string getDescription() const override { 
        return "Reactive particles that respond to audio"; 
    }
    
private:
    struct Particle {
        float x, y;           // Position
        float vx, vy;         // Velocity
        float size;           // Size
        float life;           // Life left (0.0-1.0)
        float maxLife;        // Maximum life time
        float hue;            // Color hue
        int type;             // Particle shape type
    };
    
    void updateParticles(const AudioData& audioData, float time);
    void spawnParticles(const AudioData& audioData, float time);
    void renderParticles(Renderer* renderer);
    
    std::vector<Particle> m_particles;
    int m_maxParticles;
    float m_lastTime;
    int m_width;
    int m_height;
};

/**
 * Manager class for all visualizations
 */
class VisualizationManager {
public:
    VisualizationManager();
    ~VisualizationManager();
    
    // Initialize with default visualizations
    void initialize();
    
    // Render the current visualization
    void renderCurrentVisualization(Renderer* renderer, const AudioData& audioData);
    
    // Switch to next visualization
    void nextVisualization();
    
    // Switch to previous visualization
    void previousVisualization();
    
    // Get the current visualization
    Visualization* getCurrentVisualization();
    
    // Get name of current visualization
    std::string getCurrentVisualizationName() const;
    
    // Get available visualizations
    const std::vector<std::unique_ptr<Visualization>>& getVisualizations() const { 
        return m_visualizations; 
    }
    
    // Get current visualization index
    int getCurrentIndex() const { return m_currentVisualizationIndex; }

private:
    std::vector<std::unique_ptr<Visualization>> m_visualizations;
    size_t m_currentVisualizationIndex;
};

} // namespace av 