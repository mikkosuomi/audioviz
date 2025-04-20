#include "UI.h"
#include <iostream>

namespace av {

UI::UI(Engine* engine)
    : m_engine(engine)
{
}

UI::~UI()
{
    // Button pointers are cleaned up automatically by unique_ptr
}

void UI::initialize()
{
    // Get window dimensions from engine
    int width = m_engine->getWindow()->getWidth();
    int height = m_engine->getWindow()->getHeight();
    
    // Create UI controls
    createVisualizationControls(width, height);
    createAmplificationControls(width, height);
    
    std::cout << "UI initialized with " << m_buttons.size() << " buttons" << std::endl;
}

void UI::processInput(int mouseX, int mouseY, bool mouseDown)
{
    static bool lastMouseDown = false;
    bool mouseStateChanged = mouseDown != lastMouseDown;
    static bool debugUI = false; // Set to true only when actively debugging UI issues
    
    // Log mouse state changes for debugging (only when state changes and debug is enabled)
    if (debugUI && mouseStateChanged) {
        std::cout << "UI detected mouse " << (mouseDown ? "DOWN" : "UP") << " at position " << mouseX << "," << mouseY << std::endl;
        
        // Additional debug output when mouse is pressed to identify button locations
        if (mouseDown) {
            std::cout << "Checking for button hits at " << mouseX << "," << mouseY << " with " << m_buttons.size() << " buttons:" << std::endl;
            for (size_t i = 0; i < m_buttons.size(); i++) {
                auto& btn = m_buttons[i];
                bool isHit = mouseX >= btn->getX() && mouseX <= btn->getX() + btn->getWidth() &&
                           mouseY >= btn->getY() && mouseY <= btn->getY() + btn->getHeight();
                std::cout << "  Button " << i << " (" << btn->getX() << "," << btn->getY() 
                          << " size=" << btn->getWidth() << "x" << btn->getHeight() 
                          << "): " << (isHit ? "HIT" : "miss") << std::endl;
            }
        }
        
        lastMouseDown = mouseDown;
    }
    else {
        lastMouseDown = mouseDown;
    }
    
    // Show button positions on first frame only if debugging is enabled
    static bool firstFrame = true;
    if (debugUI && firstFrame) {
        std::cout << "UI has " << m_buttons.size() << " buttons:" << std::endl;
        for (size_t i = 0; i < m_buttons.size(); i++) {
            auto& btn = m_buttons[i];
            std::cout << "  Button " << i << ": position=" << btn->getX() << "," << btn->getY() 
                      << " size=" << btn->getWidth() << "x" << btn->getHeight() << std::endl;
        }
        firstFrame = false;
    }
    else if (!debugUI) {
        firstFrame = false;
    }
    
    // Check each button for interaction
    bool buttonActivated = false;
    for (auto& button : m_buttons) {
        if (button->update(mouseX, mouseY, mouseDown)) {
            // Button was clicked and callback executed
            if (debugUI) {
                std::cout << "UI detected button activation" << std::endl;
            }
            buttonActivated = true;
            // Do not break here, let all buttons process input
        }
    }
    
    if (debugUI && buttonActivated) {
        std::cout << "Button action completed" << std::endl;
    }
}

void UI::render(Renderer* renderer)
{
    // Render all buttons
    for (auto& button : m_buttons) {
        button->render(renderer);
    }
}

void UI::onResize(int width, int height)
{
    // Clear existing buttons
    m_buttons.clear();
    
    // Recreate buttons with new size
    createVisualizationControls(width, height);
    createAmplificationControls(width, height);
    
    std::cout << "UI resized to " << width << "x" << height << std::endl;
}

void UI::createVisualizationControls(int width, int height)
{
    // Create buttons for previous/next visualization
    const int buttonSize = 50;
    const int margin = 20;
    
    // Previous visualization button (left side)
    auto prevButton = std::make_unique<Button>(
        margin, 
        height - buttonSize - margin,
        buttonSize, 
        buttonSize, 
        "<",
        Color(0.2f, 0.4f, 0.8f, 0.8f)
    );
    
    // Set callback
    prevButton->setCallback([this]() {
        std::cout << "Previous visualization button clicked" << std::endl;
        m_engine->previousVisualization();
    });
    
    // Next visualization button (right side)
    auto nextButton = std::make_unique<Button>(
        margin + buttonSize + 10, 
        height - buttonSize - margin,
        buttonSize, 
        buttonSize, 
        ">",
        Color(0.2f, 0.4f, 0.8f, 0.8f)
    );
    
    // Set callback
    nextButton->setCallback([this]() {
        std::cout << "Next visualization button clicked" << std::endl;
        m_engine->nextVisualization();
    });
    
    // Add buttons to list
    m_buttons.push_back(std::move(prevButton));
    m_buttons.push_back(std::move(nextButton));
}

void UI::createAmplificationControls(int width, int height)
{
    // Create buttons for increase/decrease amplification
    const int buttonSize = 50;
    const int margin = 20;
    
    // Increase amplification button
    auto increaseButton = std::make_unique<Button>(
        width - buttonSize - margin,
        height - buttonSize - margin,
        buttonSize,
        buttonSize,
        "+",
        Color(0.8f, 0.4f, 0.2f, 0.8f)
    );
    
    // Set callback
    increaseButton->setCallback([this]() {
        std::cout << "Increase amplification button clicked" << std::endl;
        m_engine->increaseAmplificationFactor(1.0f);
    });
    
    // Decrease amplification button
    auto decreaseButton = std::make_unique<Button>(
        width - 2 * buttonSize - margin - 10,
        height - buttonSize - margin,
        buttonSize,
        buttonSize,
        "-",
        Color(0.8f, 0.4f, 0.2f, 0.8f)
    );
    
    // Set callback
    decreaseButton->setCallback([this]() {
        std::cout << "Decrease amplification button clicked" << std::endl;
        m_engine->decreaseAmplificationFactor(1.0f);
    });
    
    // Add buttons to list
    m_buttons.push_back(std::move(increaseButton));
    m_buttons.push_back(std::move(decreaseButton));
}

} // namespace av 