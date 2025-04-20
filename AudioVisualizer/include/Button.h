#pragma once

#include "Renderer.h"
#include <string>
#include <functional>

namespace av {

class Button {
public:
    Button(int x, int y, int width, int height, const std::string& text, const Color& bgColor = Color(0.3f, 0.3f, 0.3f, 0.8f));
    
    // Update button state based on mouse position and click state
    bool update(int mouseX, int mouseY, bool mouseDown);
    
    // Render the button
    void render(Renderer* renderer);
    
    // Set the callback function to be executed when the button is clicked
    void setCallback(std::function<void()> callback);
    
    // Set text for the button
    void setText(const std::string& text);
    
    // Set position
    void setPosition(int x, int y);
    
    // Set dimensions
    void setSize(int width, int height);
    
    // Get position and dimensions
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::string m_text;
    Color m_backgroundColor;
    Color m_hoverColor;
    Color m_pressedColor;
    Color m_textColor;
    bool m_isHovered;
    bool m_isPressed;
    std::function<void()> m_callback;
};

} // namespace av 