#pragma once

#include <array>
#include <unordered_map>
#include <vector>

namespace av {

// Forward declarations
class Window;

/**
 * Mouse button IDs
 */
enum class MouseButton {
    Left,
    Middle,
    Right,
    X1,
    X2,
    Count
};

/**
 * Input event types
 */
enum class EventType {
    KeyDown,
    KeyUp,
    MouseButtonDown,
    MouseButtonUp,
    MouseMove,
    MouseWheel,
    WindowResize,
    WindowClose,
    WindowFocus,
    WindowBlur,
    Quit
};

/**
 * Input event data
 */
struct InputEvent {
    EventType type;
    
    union {
        struct {
            int keyCode;
            bool repeat;
        } key;
        
        struct {
            MouseButton button;
            int x, y;
            int clicks;
        } mouseButton;
        
        struct {
            int x, y;
            int relX, relY;
        } mouseMove;
        
        struct {
            int x, y;
        } mouseWheel;
        
        struct {
            int width, height;
        } windowResize;
    };
};

/**
 * Handles all user input
 */
class InputManager {
public:
    InputManager(Window* window);
    ~InputManager();

    // Process all pending input events
    void processEvents();
    
    // Key state
    bool isKeyDown(int keyCode) const;
    bool isKeyPressed(int keyCode) const;
    bool isKeyReleased(int keyCode) const;
    
    // Mouse state
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    
    // Mouse position
    int getMouseX() const { return m_mouseX; }
    int getMouseY() const { return m_mouseY; }
    
    // Track drag operations
    void beginDrag(int mouseX, int mouseY, int windowX, int windowY);
    void endDrag();
    bool isDragging() const { return m_dragging; }
    
    // Get drag information for window movement
    void getDragStartPositions(int& outMouseX, int& outMouseY, int& outWindowX, int& outWindowY) const;
    
    // For backward compatibility, will be removed
    void getDragDelta(int& outDeltaX, int& outDeltaY) const;
    
    // Check for double clicks
    bool isDoubleClick(MouseButton button, int interval = 300) const;
    
    // Get all input events for this frame
    const std::vector<InputEvent>& getEvents() const { return m_events; }
    
    // Utility
    void clearEvents() { m_events.clear(); }

private:
    Window* m_window;
    
    // Track all relevant input states
    std::array<bool, 512> m_currentKeys;
    std::array<bool, 512> m_previousKeys;
    std::array<bool, static_cast<int>(MouseButton::Count)> m_currentMouseButtons;
    std::array<bool, static_cast<int>(MouseButton::Count)> m_previousMouseButtons;
    
    // Mouse data
    int m_mouseX;
    int m_mouseY;
    int m_mouseWheelX;
    int m_mouseWheelY;
    
    // Drag support
    bool m_dragging;
    int m_dragStartMouseX;
    int m_dragStartMouseY;
    int m_dragStartWindowX;
    int m_dragStartWindowY;
    
    // Double-click support
    std::array<unsigned int, static_cast<int>(MouseButton::Count)> m_lastClickTimes;
    
    // Current frame's events
    std::vector<InputEvent> m_events;
    
    // Window position at drag start (for smoother dragging)
    int m_dragWindowStartX;
    int m_dragWindowStartY;
    
    // Update press/release state
    void updateKeyStates();
    void updateMouseButtonStates();
};

} // namespace av 