#include "Engine.h"
#include <iostream>
#include <string>
#include <SDL.h>

// Default settings
constexpr int DEFAULT_WIDTH = 1024;
constexpr int DEFAULT_HEIGHT = 768;
const std::string DEFAULT_TITLE = "Audio Visualizer";

int main(int argc, char* argv[]) {
    std::cout << "====================================================================================\n";
    std::cout << "================== STARTING APPLICATION - SUPER DEBUG VERSION =====================\n";
    std::cout << "====================================================================================\n";
    std::cout << "Window size: " << DEFAULT_WIDTH << "x" << DEFAULT_HEIGHT << std::endl;
    std::cout << "OpenGL version requested: 2.1 (compatibility profile)" << std::endl;
    std::cout << "Rendering approach: DIRECT RENDERING (bypassing framebuffers)" << std::endl;
    
    // Print system info
    std::cout << "SDL Version: " << SDL_MAJOR_VERSION << "." 
              << SDL_MINOR_VERSION << "." << SDL_PATCHLEVEL << std::endl;
    
    // Create the engine
    av::Engine engine;
    
    // Initialize the engine
    std::cout << "Calling engine.initialize()..." << std::endl;
    if (!engine.initialize(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_TITLE)) {
        std::cerr << "Failed to initialize the engine!" << std::endl;
        return 1;
    }
    
    // Skip script loading and use direct rendering
    std::cout << "Starting engine main loop - Window should appear with RED SCREEN and WHITE X..." << std::endl;
    std::cout << "Press ESC key to exit" << std::endl;
    std::cout << "Using DIRECT RENDERING with bright red background and white X pattern" << std::endl;
    
    // Skip the fullscreen toggle - it may be causing issues
    // std::cout << "Toggling fullscreen to see if it resolves display issues..." << std::endl;
    // engine.toggleFullscreen();
    
    // Run the engine
    std::cout << "Engine.run() - Starting main loop NOW" << std::endl;
    engine.run();
    
    // Cleanup
    std::cout << "Engine.run() completed, shutting down..." << std::endl;
    engine.shutdown();
    
    std::cout << "Application completed successfully" << std::endl;
    return 0;
} 