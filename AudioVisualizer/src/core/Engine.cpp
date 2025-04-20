#include "Engine.h"
#include <iostream>
#include <SDL.h>
#include <cmath>
#include "visualizations/SimpleVisualizer.h"

namespace av {

Engine::Engine()
    : m_isRunning(false)
    , m_lastFrameTime(0.0)
    , m_deltaTime(0.0)
    , m_simpleVisualizer(nullptr)
{
    std::cout << "Audio Visualizer Engine created" << std::endl;
}

Engine::~Engine()
{
    shutdown();
}

bool Engine::initialize(int width, int height, const std::string& title)
{
    std::cout << "Initializing Audio Visualizer Engine..." << std::endl;
    
    // Create window
    m_window = std::make_unique<Window>();
    if (!m_window->initialize(width, height, title)) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }
    
    // Create input manager
    m_inputManager = std::make_unique<InputManager>(m_window.get());
    
    // Create audio processor
    m_audioProcessor = std::make_unique<AudioProcessor>();
    if (!m_audioProcessor->initialize()) {
        std::cerr << "Warning: Failed to initialize audio processor" << std::endl;
        // Continue anyway, audio might not be available
    }
    
    // Create renderer
    m_renderer = std::make_unique<Renderer>(m_window.get());
    if (!m_renderer->initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // Create script engine
    m_scriptEngine = std::make_unique<ScriptEngine>(this);
    if (!m_scriptEngine->initialize()) {
        std::cerr << "Warning: Failed to initialize script engine" << std::endl;
        // Continue anyway, we can run without scripts
    }

    // Initialize SimpleVisualizer
    m_simpleVisualizer = std::make_unique<SimpleVisualizer>();
    // Set initial size
    m_simpleVisualizer->onResize(width, height);
    
    m_isRunning = true;
    m_lastFrameTime = 0.0;
    
    std::cout << "Engine initialized successfully" << std::endl;
    return true;
}

void Engine::run()
{
    std::cout << "Engine running..." << std::endl;
    
    m_lastFrameTime = SDL_GetTicks() / 1000.0;
    int frameCount = 0;
    double lastFpsTime = m_lastFrameTime;
    
    while (m_isRunning) {
        processFrame();
        
        // Print FPS every 1 second
        frameCount++;
        double currentTime = SDL_GetTicks() / 1000.0;
        if (currentTime - lastFpsTime > 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastFpsTime = currentTime;
        }
        
        // Add a small delay to avoid running too fast
        SDL_Delay(10);
    }
}

void Engine::shutdown()
{
    std::cout << "Shutting down engine..." << std::endl;
    
    // Shutdown simple visualizer
    if (m_simpleVisualizer) {
        m_simpleVisualizer->cleanup();
    }
    
    // Shutdown script engine first
    if (m_scriptEngine) {
        m_scriptEngine->shutdown();
    }
    
    // Shutdown renderer
    if (m_renderer) {
        m_renderer->shutdown();
    }
    
    // Shutdown audio processor
    if (m_audioProcessor) {
        m_audioProcessor->shutdown();
    }
    
    // Shutdown window last
    if (m_window) {
        m_window->shutdown();
    }
    
    m_isRunning = false;
    std::cout << "Engine shutdown complete" << std::endl;
}

bool Engine::loadVisualization(const std::string& scriptPath)
{
    if (!m_scriptEngine) {
        std::cerr << "Script engine not initialized" << std::endl;
        return false;
    }
    
    if (!m_scriptEngine->loadScript(scriptPath)) {
        std::cerr << "Failed to load script: " << scriptPath << std::endl;
        return false;
    }
    
    // Initialize the script
    if (!m_scriptEngine->onInit()) {
        std::cerr << "Failed to initialize script: " << scriptPath << std::endl;
        return false;
    }
    
    std::cout << "Visualization loaded: " << scriptPath << std::endl;
    return true;
}

void Engine::toggleFullscreen()
{
    if (m_window) {
        m_window->toggleFullscreen();
    }
}

void Engine::processFrame()
{
    // Calculate delta time
    double currentTime = SDL_GetTicks() / 1000.0;
    m_deltaTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;
    
    // Cap delta time to prevent large jumps
    if (m_deltaTime > 0.1) {
        m_deltaTime = 0.1;
    }
    
    // Process input
    m_inputManager->processEvents();
    
    // Check for quit event
    for (const auto& event : m_inputManager->getEvents()) {
        if (event.type == EventType::Quit || 
            (event.type == EventType::KeyDown && event.key.keyCode == SDLK_ESCAPE)) {
            m_isRunning = false;
            return;
        }
        
        // Handle left mouse button for dragging in borderless mode
        if (m_window->isBorderless()) {
            // Start dragging on left mouse button down
            if (event.type == EventType::MouseButtonDown && 
                event.mouseButton.button == MouseButton::Left) {
                // Store initial window position when drag starts
                int windowX, windowY;
                SDL_GetWindowPosition(m_window->getSDLWindow(), &windowX, &windowY);
                
                // Store both mouse and window positions for relative movement
                m_inputManager->beginDrag(event.mouseButton.x, event.mouseButton.y, windowX, windowY);
            }
            
            // End dragging on left mouse button up
            if (event.type == EventType::MouseButtonUp && 
                event.mouseButton.button == MouseButton::Left) {
                m_inputManager->endDrag();
            }
            
            // Check for double-click to toggle fullscreen
            if (event.type == EventType::MouseButtonDown && 
                event.mouseButton.button == MouseButton::Left &&
                event.mouseButton.clicks == 2) {
                m_window->toggleFullscreen();
                
                // Update renderer with new window size
                resizeRenderer();
            }
            
            // Move window while dragging - use relative movement from initial positions
            if (m_inputManager->isDragging() && event.type == EventType::MouseMove) {
                // Get initial positions
                int startMouseX, startMouseY, startWindowX, startWindowY;
                m_inputManager->getDragStartPositions(startMouseX, startMouseY, startWindowX, startWindowY);
                
                // Calculate movement delta from initial mouse position
                int deltaX = event.mouseMove.x - startMouseX;
                int deltaY = event.mouseMove.y - startMouseY;
                
                // Move window based on initial window position plus the delta
                SDL_SetWindowPosition(m_window->getSDLWindow(), 
                                   startWindowX + deltaX, 
                                   startWindowY + deltaY);
            }
        }
        
        // Handle window resize events
        if (event.type == EventType::WindowResize) {
            resizeRenderer();
        }
    }
    
    // Process audio
    m_audioProcessor->update();
    
    // Update script
    const AudioData& audioData = m_audioProcessor->getAudioData();
    if (m_scriptEngine && m_scriptEngine->isScriptLoaded()) {
        m_scriptEngine->onUpdate(m_deltaTime, audioData);
    }
    
    // This section is enabled now - use the Renderer
    
    // Render frame
    m_renderer->beginFrame();
    
    if (m_scriptEngine && m_scriptEngine->isScriptLoaded()) {
        m_scriptEngine->onRender(m_renderer.get());
    }
    else if (m_simpleVisualizer) {
        // Use the SimpleVisualizer instead of the default visualization
        m_simpleVisualizer->render(m_renderer.get(), audioData);
    }
    else {
        // Fallback to default visualization if SimpleVisualizer fails
        renderDefaultVisualization(audioData);
    }
    
    // Add this line to complete rendering and swap buffers
    m_renderer->endFrame();
}

// Add a helper method to resize the renderer
void Engine::resizeRenderer()
{
    if (m_renderer) {
        int width = m_window->getWidth();
        int height = m_window->getHeight();
        
        // Notify renderer of size change
        m_renderer->resize(width, height);
        
        // Notify simple visualizer of size change
        if (m_simpleVisualizer) {
            m_simpleVisualizer->onResize(width, height);
        }
        
        std::cout << "Resized renderer to " << width << "x" << height << std::endl;
    }
}

// Add a new method to render a default visualization
void Engine::renderDefaultVisualization(const AudioData& audioData)
{
    static bool firstRender = true;
    static int renderCount = 0;
    
    renderCount++;
    
    if (firstRender) {
        std::cout << "Rendering default visualization" << std::endl;
        firstRender = false;
    }
    
    if (renderCount % 100 == 0) {
        std::cout << "Default visualization render count: " << renderCount 
                  << " | Waveform size: " << audioData.waveform.size()
                  << " | Spectrum size: " << audioData.spectrum.size() << std::endl;
        
        // Print first few samples of waveform to verify data
        if (!audioData.waveform.empty()) {
            std::cout << "Waveform samples: ";
            for (int i = 0; i < std::min(5, (int)audioData.waveform.size()); i++) {
                std::cout << audioData.waveform[i] << " ";
            }
            std::cout << std::endl;
        }
    }
    
    int width = m_window->getWidth();
    int height = m_window->getHeight();
    
    // Draw background
    float time = static_cast<float>(m_lastFrameTime);
    Color bgColor = Color::fromHSV(time * 0.1f, 0.2f, 0.2f);
    m_renderer->drawFilledRect(0, 0, width, height, bgColor);
    
    // Draw audio waveform with more prominence
    Color waveColor = Color::fromHSV(time * 0.1f + 0.5f, 0.9f, 0.9f);
    
    // Create a background box for the waveform
    Color waveformBgColor(0.05f, 0.05f, 0.2f, 1.0f);
    float waveHeight = height / 2.0f; // Use half the screen height
    float waveTop = 50.0f;
    m_renderer->drawFilledRect(20, waveTop, width - 40, waveHeight, waveformBgColor);
    m_renderer->drawRect(20, waveTop, width - 40, waveHeight, Color(0.5f, 0.5f, 1.0f, 0.8f), 2.0f);
    
    // Draw center line
    m_renderer->drawLine(20, waveTop + waveHeight/2, width - 20, waveTop + waveHeight/2, 
                       Color(0.7f, 0.7f, 0.7f, 0.5f), 1.0f);
    
    // Draw the waveform with larger size
    m_renderer->drawWaveform(audioData.waveform.data(), audioData.waveform.size(), 
                          20, waveTop, width - 40, waveHeight, waveColor);
    
    // Draw audio spectrum below waveform
    Color spectrumColor = Color::fromHSV(time * 0.1f + 0.2f, 0.8f, 1.0f);
    m_renderer->drawSpectrum(audioData.spectrum.data(), audioData.spectrum.size(), 
                          20, waveTop + waveHeight + 20, width - 40, height / 5, spectrumColor);
    
    // Draw reactive circles based on audio energy - move to bottom of screen
    float bassRadius = 50.0f + audioData.bass * 100.0f;
    float midRadius = 30.0f + audioData.mid * 70.0f;
    float trebleRadius = 15.0f + audioData.treble * 40.0f;
    
    int circleY = height - 80;
    
    m_renderer->drawFilledCircle(width / 4, circleY, bassRadius, 
                               Color::fromHSV(time * 0.2f, 0.7f, 0.6f, 0.6f));
    m_renderer->drawFilledCircle(width / 2, circleY, midRadius, 
                               Color::fromHSV(time * 0.2f + 0.33f, 0.7f, 0.7f, 0.7f));
    m_renderer->drawFilledCircle(width * 3 / 4, circleY, trebleRadius, 
                               Color::fromHSV(time * 0.2f + 0.66f, 0.7f, 0.8f, 0.8f));
    
    // Draw energy meter at bottom
    float energyWidth = width * 0.8f;
    float meterHeight = 20.0f;
    float meterX = (width - energyWidth) / 2.0f;
    float meterY = height - 30.0f;
    
    m_renderer->drawFilledRect(meterX, meterY, energyWidth, meterHeight, Color(0.1f, 0.1f, 0.1f, 0.8f));
    m_renderer->drawFilledRect(meterX, meterY, energyWidth * audioData.energy, meterHeight, 
                             Color(1.0f, 0.0f, 0.0f, 0.8f));
    m_renderer->drawRect(meterX, meterY, energyWidth, meterHeight, Color(1.0f, 1.0f, 1.0f, 0.8f), 1.0f);
}

} // namespace av 