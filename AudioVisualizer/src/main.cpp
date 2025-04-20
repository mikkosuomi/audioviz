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
    std::cout << "================== STARTING APPLICATION - ENHANCED VISUALIZERS ====================\n";
    std::cout << "====================================================================================\n";
    std::cout << "Window size: " << DEFAULT_WIDTH << "x" << DEFAULT_HEIGHT << std::endl;
    std::cout << "OpenGL version requested: 2.1 (compatibility profile)" << std::endl;
    std::cout << "Rendering approach: DIRECT RENDERING (bypassing framebuffers)" << std::endl;
    
    // Print system info
    std::cout << "SDL Version: " << SDL_MAJOR_VERSION << "." 
              << SDL_MINOR_VERSION << "." << SDL_PATCHLEVEL << std::endl;
    
    // Print available visualizers
    std::cout << "\nAvailable visualizers:\n";
    std::cout << "1. Simple Visualizer - Classic frequency spectrum and waveform\n";
    std::cout << "2. Matrix Visualizer - Digital rain effect inspired by The Matrix\n";
    std::cout << "3. 3D Bars Visualizer - 3D bars that react to frequency spectrum\n";
    std::cout << "4. Particle Fountain - Audio-reactive particle fountain\n";
    
    // Print controls
    std::cout << "\nControls:\n";
    std::cout << "- Press ESC to exit\n";
    std::cout << "- Use LEFT/RIGHT arrow keys to switch between visualizers\n";
    std::cout << "- Press F11 to toggle fullscreen mode\n";
    
    // Create the engine
    av::Engine engine;
    
    // Initialize the engine
    std::cout << "Calling engine.initialize()..." << std::endl;
    if (!engine.initialize(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_TITLE)) {
        std::cerr << "Failed to initialize the engine!" << std::endl;
        return 1;
    }
    
    // Run the engine
    std::cout << "Engine.run() - Starting main loop NOW" << std::endl;
    engine.run();
    
    return 0;
} 