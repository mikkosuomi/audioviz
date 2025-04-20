#pragma once

#include "Button.h"
#include "Engine.h"
#include <vector>
#include <memory>

namespace av {

class UI {
public:
    UI(Engine* engine);
    ~UI();
    
    // Initialize the UI
    void initialize();
    
    // Process UI input
    void processInput(int mouseX, int mouseY, bool mouseDown);
    
    // Render the UI
    void render(Renderer* renderer);
    
    // Resize UI elements when window size changes
    void onResize(int width, int height);
    
private:
    Engine* m_engine;
    std::vector<std::unique_ptr<Button>> m_buttons;
    
    // Button creation helpers
    void createVisualizationControls(int width, int height);
    void createAmplificationControls(int width, int height);
};

} // namespace av 