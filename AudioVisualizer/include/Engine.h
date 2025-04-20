#pragma once

#include "Window.h"
#include "InputManager.h"
#include "AudioProcessor.h"
#include "Renderer.h"
#include "ScriptEngine.h"
#include "Visualization.h"
#include "visualizations/SimpleVisualizer.h"

#include <memory>
#include <string>

namespace av {

/**
 * Main engine class that coordinates all systems
 */
class Engine {
public:
    Engine();
    ~Engine();

    // Initialize the engine
    bool initialize(int width, int height, const std::string& title);
    
    // Main run loop
    void run();
    
    // Shutdown the engine
    void shutdown();
    
    // Load a visualization script
    bool loadVisualization(const std::string& scriptPath);
    
    // Switch to next/previous built-in visualization
    void nextVisualization();
    void previousVisualization();
    
    // Toggle fullscreen mode
    void toggleFullscreen();
    
    // Getters for subsystems
    Window* getWindow() { return m_window.get(); }
    InputManager* getInputManager() { return m_inputManager.get(); }
    AudioProcessor* getAudioProcessor() { return m_audioProcessor.get(); }
    Renderer* getRenderer() { return m_renderer.get(); }
    ScriptEngine* getScriptEngine() { return m_scriptEngine.get(); }
    VisualizationManager* getVisualizationManager() { return m_visualizationManager.get(); }
    SimpleVisualizer* getSimpleVisualizer() { return m_simpleVisualizer.get(); }

    // Get audio data for scripts
    const AudioData& getAudioData() const { return m_audioProcessor->getAudioData(); }

private:
    // Process a single frame
    void processFrame();
    
    // Render a default visualization when no script is loaded
    void renderDefaultVisualization(const AudioData& audioData);
    
    // Resize the renderer
    void resizeRenderer();
    
    // Engine subsystems
    std::unique_ptr<Window> m_window;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<AudioProcessor> m_audioProcessor;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ScriptEngine> m_scriptEngine;
    std::unique_ptr<VisualizationManager> m_visualizationManager;
    std::unique_ptr<SimpleVisualizer> m_simpleVisualizer;
    
    // Engine state
    bool m_isRunning;
    double m_lastFrameTime;
    double m_deltaTime;
    bool m_useBuiltInVisualizations;
};

} // namespace av 