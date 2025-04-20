#include "Button.h"
#include <iostream>

namespace av {

Button::Button(int x, int y, int width, int height, const std::string& text, const Color& bgColor)
    : m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_text(text)
    , m_backgroundColor(bgColor)
    , m_isHovered(false)
    , m_isPressed(false)
{
    // Create hover color (significantly lighter than background for better visibility)
    m_hoverColor = Color(
        std::min(m_backgroundColor.r + 0.2f, 1.0f),
        std::min(m_backgroundColor.g + 0.2f, 1.0f),
        std::min(m_backgroundColor.b + 0.2f, 1.0f),
        m_backgroundColor.a
    );
    
    // Create pressed color (significantly darker than background for better feedback)
    m_pressedColor = Color(
        std::max(m_backgroundColor.r - 0.2f, 0.0f),
        std::max(m_backgroundColor.g - 0.2f, 0.0f),
        std::max(m_backgroundColor.b - 0.2f, 0.0f),
        m_backgroundColor.a
    );
    
    // Set text color (white by default)
    m_textColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

bool Button::update(int mouseX, int mouseY, bool mouseDown)
{
    // Debug flag - only enable when debugging button issues
    static bool debugButtons = false;
    
    // Check if mouse is over the button
    bool isMouseOver = mouseX >= m_x && mouseX <= m_x + m_width &&
                      mouseY >= m_y && mouseY <= m_y + m_height;
    
    // Debug output when mouse state changes on this button
    static bool wasMouseOver = false;
    if (debugButtons && isMouseOver != wasMouseOver) {
        std::cout << "Button '" << m_text << "' mouse " << (isMouseOver ? "ENTERED" : "EXITED") 
                  << " at position " << mouseX << "," << mouseY 
                  << " (button is at " << m_x << "," << m_y << " with size " << m_width << "x" << m_height << ")" << std::endl;
        wasMouseOver = isMouseOver;
    }
    else {
        wasMouseOver = isMouseOver;
    }
    
    // Track mouse state changes
    static bool wasMouseDown = false;
    bool mouseStateChanged = mouseDown != wasMouseDown;
    wasMouseDown = mouseDown;
    
    // Update hover state
    m_isHovered = isMouseOver;
    
    // Mouse was pressed down on this button
    if (isMouseOver && mouseDown && !m_isPressed) {
        m_isPressed = true;
        if (debugButtons) {
            std::cout << "Button '" << m_text << "' pressed" << std::endl;
        }
        return false; // Not activated yet, just pressed
    }
    
    // Mouse was released after being pressed
    if (m_isPressed && !mouseDown) {
        if (debugButtons) {
            std::cout << "Button '" << m_text << "' released" << std::endl;
        }
        m_isPressed = false;
        
        // Only trigger if releasing over the button
        if (isMouseOver && m_callback) {
            if (debugButtons) {
                std::cout << "Button '" << m_text << "' activated, executing callback" << std::endl;
            }
            m_callback();
            return true;
        } else if (debugButtons) {
            std::cout << "Button '" << m_text << "' released but not activated" 
                      << (isMouseOver ? " (mouse still over button)" : " (mouse not over button)")
                      << (m_callback ? " (has callback)" : " (no callback)") << std::endl;
        }
    }
    
    return false;
}

void Button::render(Renderer* renderer)
{
    // Determine which color to use
    Color buttonColor;
    if (m_isPressed) {
        buttonColor = m_pressedColor;
    }
    else if (m_isHovered) {
        buttonColor = m_hoverColor;
    }
    else {
        buttonColor = m_backgroundColor;
    }
    
    // Draw button background
    renderer->drawFilledRect(m_x, m_y, m_width, m_height, buttonColor);
    
    // Draw button border with thicker outline when hovered or pressed
    float borderThickness = (m_isHovered || m_isPressed) ? 2.0f : 1.0f;
    renderer->drawRect(m_x, m_y, m_width, m_height, Color(1.0f, 1.0f, 1.0f, 0.8f), borderThickness);
    
    // Draw text centered on button
    // Note: Since we don't have text rendering directly in Renderer, we'll add a placeholder
    // You'll need to implement text rendering in your actual application
    
    // Draw a small indicator line to represent text
    float centerX = m_x + m_width / 2.0f;
    float centerY = m_y + m_height / 2.0f;
    float lineThickness = (m_isHovered || m_isPressed) ? 2.5f : 2.0f;
    
    renderer->drawLine(centerX - 10, centerY, centerX + 10, centerY, m_textColor, lineThickness);
    
    // Draw a perpendicular line if this is a next/prev button
    if (m_text == "Next" || m_text == ">") {
        renderer->drawLine(centerX + 5, centerY - 5, centerX + 10, centerY, m_textColor, lineThickness);
        renderer->drawLine(centerX + 5, centerY + 5, centerX + 10, centerY, m_textColor, lineThickness);
    }
    else if (m_text == "Prev" || m_text == "<") {
        renderer->drawLine(centerX - 5, centerY - 5, centerX - 10, centerY, m_textColor, lineThickness);
        renderer->drawLine(centerX - 5, centerY + 5, centerX - 10, centerY, m_textColor, lineThickness);
    }
    else if (m_text == "+" || m_text == "Up") {
        renderer->drawLine(centerX, centerY - 10, centerX, centerY + 10, m_textColor, lineThickness);
        renderer->drawLine(centerX - 10, centerY, centerX + 10, centerY, m_textColor, lineThickness);
    }
    else if (m_text == "-" || m_text == "Down") {
        renderer->drawLine(centerX - 10, centerY, centerX + 10, centerY, m_textColor, lineThickness);
    }
}

void Button::setCallback(std::function<void()> callback)
{
    m_callback = callback;
}

void Button::setText(const std::string& text)
{
    m_text = text;
}

void Button::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
}

void Button::setSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

} // namespace av 