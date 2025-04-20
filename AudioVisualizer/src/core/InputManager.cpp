#include "InputManager.h"
#include "Window.h"
#include <SDL.h>
#include <iostream>

namespace av {

InputManager::InputManager(Window* window)
    : m_window(window)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_mouseWheelX(0)
    , m_mouseWheelY(0)
    , m_dragging(false)
    , m_dragStartMouseX(0)
    , m_dragStartMouseY(0)
    , m_dragStartWindowX(0)
    , m_dragStartWindowY(0)
{
    std::fill(m_currentKeys.begin(), m_currentKeys.end(), false);
    std::fill(m_previousKeys.begin(), m_previousKeys.end(), false);
    std::fill(m_currentMouseButtons.begin(), m_currentMouseButtons.end(), false);
    std::fill(m_previousMouseButtons.begin(), m_previousMouseButtons.end(), false);
    std::fill(m_lastClickTimes.begin(), m_lastClickTimes.end(), 0);
}

InputManager::~InputManager()
{
}

void InputManager::processEvents()
{
    // Save current state as previous
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;
    
    // Clear events for this frame
    m_events.clear();
    
    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                InputEvent quitEvent;
                quitEvent.type = EventType::Quit;
                m_events.push_back(quitEvent);
                break;
            }
            
            case SDL_KEYDOWN: {
                // Special debugging for arrow keys
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT ||
                    event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                    std::cout << "ARROW KEY DETECTED IN INPUT MANAGER: " << event.key.keysym.sym << std::endl;
                    std::cout << "SDLK_LEFT=" << SDLK_LEFT << " SDLK_RIGHT=" << SDLK_RIGHT 
                             << " SDLK_UP=" << SDLK_UP << " SDLK_DOWN=" << SDLK_DOWN << std::endl;
                }
                
                if (event.key.keysym.sym < 512) {
                    m_currentKeys[event.key.keysym.sym] = true;
                    
                    // Log SDL key events for debugging
                    std::cout << "SDL_KEYDOWN event detected - Key code: " << event.key.keysym.sym 
                              << ", Scancode: " << event.key.keysym.scancode << std::endl;
                    
                    InputEvent keyEvent;
                    keyEvent.type = EventType::KeyDown;
                    keyEvent.key.keyCode = event.key.keysym.sym;
                    keyEvent.key.repeat = event.key.repeat != 0;
                    m_events.push_back(keyEvent);
                }
                break;
            }
            
            case SDL_KEYUP: {
                if (event.key.keysym.sym < 512) {
                    m_currentKeys[event.key.keysym.sym] = false;
                    
                    // Log SDL key events for debugging
                    std::cout << "SDL_KEYUP event detected - Key code: " << event.key.keysym.sym 
                              << ", Scancode: " << event.key.keysym.scancode << std::endl;
                    
                    InputEvent keyEvent;
                    keyEvent.type = EventType::KeyUp;
                    keyEvent.key.keyCode = event.key.keysym.sym;
                    keyEvent.key.repeat = false;
                    m_events.push_back(keyEvent);
                }
                break;
            }
            
            case SDL_MOUSEMOTION: {
                m_mouseX = event.motion.x;
                m_mouseY = event.motion.y;
                
                InputEvent mouseEvent;
                mouseEvent.type = EventType::MouseMove;
                mouseEvent.mouseMove.x = event.motion.x;
                mouseEvent.mouseMove.y = event.motion.y;
                mouseEvent.mouseMove.relX = event.motion.xrel;
                mouseEvent.mouseMove.relY = event.motion.yrel;
                m_events.push_back(mouseEvent);
                break;
            }
            
            case SDL_MOUSEBUTTONDOWN: {
                MouseButton button;
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT: button = MouseButton::Left; break;
                    case SDL_BUTTON_MIDDLE: button = MouseButton::Middle; break;
                    case SDL_BUTTON_RIGHT: button = MouseButton::Right; break;
                    case SDL_BUTTON_X1: button = MouseButton::X1; break;
                    case SDL_BUTTON_X2: button = MouseButton::X2; break;
                    default: continue;  // Skip unknown buttons
                }
                
                m_currentMouseButtons[static_cast<int>(button)] = true;
                
                InputEvent mouseEvent;
                mouseEvent.type = EventType::MouseButtonDown;
                mouseEvent.mouseButton.button = button;
                mouseEvent.mouseButton.x = event.button.x;
                mouseEvent.mouseButton.y = event.button.y;
                mouseEvent.mouseButton.clicks = event.button.clicks;
                m_events.push_back(mouseEvent);
                
                // Record click time for double-click detection
                m_lastClickTimes[static_cast<int>(button)] = SDL_GetTicks();
                break;
            }
            
            case SDL_MOUSEBUTTONUP: {
                MouseButton button;
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT: button = MouseButton::Left; break;
                    case SDL_BUTTON_MIDDLE: button = MouseButton::Middle; break;
                    case SDL_BUTTON_RIGHT: button = MouseButton::Right; break;
                    case SDL_BUTTON_X1: button = MouseButton::X1; break;
                    case SDL_BUTTON_X2: button = MouseButton::X2; break;
                    default: continue;  // Skip unknown buttons
                }
                
                m_currentMouseButtons[static_cast<int>(button)] = false;
                
                InputEvent mouseEvent;
                mouseEvent.type = EventType::MouseButtonUp;
                mouseEvent.mouseButton.button = button;
                mouseEvent.mouseButton.x = event.button.x;
                mouseEvent.mouseButton.y = event.button.y;
                mouseEvent.mouseButton.clicks = event.button.clicks;
                m_events.push_back(mouseEvent);
                break;
            }
            
            case SDL_MOUSEWHEEL: {
                m_mouseWheelX = event.wheel.x;
                m_mouseWheelY = event.wheel.y;
                
                InputEvent wheelEvent;
                wheelEvent.type = EventType::MouseWheel;
                wheelEvent.mouseWheel.x = event.wheel.x;
                wheelEvent.mouseWheel.y = event.wheel.y;
                m_events.push_back(wheelEvent);
                break;
            }
            
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        InputEvent resizeEvent;
                        resizeEvent.type = EventType::WindowResize;
                        resizeEvent.windowResize.width = event.window.data1;
                        resizeEvent.windowResize.height = event.window.data2;
                        m_events.push_back(resizeEvent);
                        break;
                    }
                    
                    case SDL_WINDOWEVENT_CLOSE: {
                        InputEvent closeEvent;
                        closeEvent.type = EventType::WindowClose;
                        m_events.push_back(closeEvent);
                        break;
                    }
                    
                    case SDL_WINDOWEVENT_FOCUS_GAINED: {
                        InputEvent focusEvent;
                        focusEvent.type = EventType::WindowFocus;
                        m_events.push_back(focusEvent);
                        break;
                    }
                    
                    case SDL_WINDOWEVENT_FOCUS_LOST: {
                        InputEvent blurEvent;
                        blurEvent.type = EventType::WindowBlur;
                        m_events.push_back(blurEvent);
                        break;
                    }
                }
                break;
            }
        }
    }
}

bool InputManager::isKeyDown(int keyCode) const
{
    return (keyCode < 512) ? m_currentKeys[keyCode] : false;
}

bool InputManager::isKeyPressed(int keyCode) const
{
    return (keyCode < 512) ? (m_currentKeys[keyCode] && !m_previousKeys[keyCode]) : false;
}

bool InputManager::isKeyReleased(int keyCode) const
{
    return (keyCode < 512) ? (!m_currentKeys[keyCode] && m_previousKeys[keyCode]) : false;
}

bool InputManager::isMouseButtonDown(MouseButton button) const
{
    return m_currentMouseButtons[static_cast<int>(button)];
}

bool InputManager::isMouseButtonPressed(MouseButton button) const
{
    return m_currentMouseButtons[static_cast<int>(button)] && !m_previousMouseButtons[static_cast<int>(button)];
}

bool InputManager::isMouseButtonReleased(MouseButton button) const
{
    return !m_currentMouseButtons[static_cast<int>(button)] && m_previousMouseButtons[static_cast<int>(button)];
}

void InputManager::beginDrag(int mouseX, int mouseY, int windowX, int windowY)
{
    m_dragging = true;
    m_dragStartMouseX = mouseX;
    m_dragStartMouseY = mouseY;
    m_dragStartWindowX = windowX;
    m_dragStartWindowY = windowY;
}

void InputManager::endDrag()
{
    m_dragging = false;
}

void InputManager::getDragStartPositions(int& outMouseX, int& outMouseY, int& outWindowX, int& outWindowY) const
{
    outMouseX = m_dragStartMouseX;
    outMouseY = m_dragStartMouseY;
    outWindowX = m_dragStartWindowX;
    outWindowY = m_dragStartWindowY;
}

void InputManager::getDragDelta(int& outDeltaX, int& outDeltaY) const
{
    outDeltaX = m_mouseX - m_dragStartMouseX;
    outDeltaY = m_mouseY - m_dragStartMouseY;
}

bool InputManager::isDoubleClick(MouseButton button, int interval) const
{
    const unsigned int buttonIndex = static_cast<unsigned int>(button);
    if (buttonIndex >= m_lastClickTimes.size()) {
        return false;
    }
    
    for (const auto& event : m_events) {
        if (event.type == EventType::MouseButtonDown && 
            event.mouseButton.button == button && 
            event.mouseButton.clicks == 2) {
            return true;
        }
    }
    
    return false;
}

void InputManager::updateKeyStates()
{
    m_previousKeys = m_currentKeys;
}

void InputManager::updateMouseButtonStates()
{
    m_previousMouseButtons = m_currentMouseButtons;
}

} // namespace av 