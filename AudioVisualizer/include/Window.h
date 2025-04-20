#pragma once

#include <string>

// Forward declare SDL types
struct SDL_Window;
typedef void* SDL_GLContext;

namespace av {

/**
 * Manages the application window and OpenGL context
 */
class Window {
public:
    Window();
    ~Window();

    // Initialize and destroy
    bool initialize(int width, int height, const std::string& title);
    void shutdown();
    
    // Update the window
    void swapBuffers();
    
    // Get window dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Window state management
    void toggleFullscreen();
    void setWindowPosition(int x, int y);
    void setWindowSize(int width, int height);
    
    // Access to SDL window for drag operations
    SDL_Window* getSDLWindow() const;
    bool isBorderless() const;

private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    
    int m_width;
    int m_height;
    
    // Fullscreen management
    int m_savedPosX;
    int m_savedPosY;
    int m_savedWidth;
    int m_savedHeight;
    
    // Window state
    bool m_fullscreen;
    bool m_minimized;
    bool m_borderless;
};

} // namespace av 