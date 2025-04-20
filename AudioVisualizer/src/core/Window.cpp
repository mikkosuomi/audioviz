#include "Window.h"
#include <iostream>

// Include SDL
#include <SDL.h>
#include <GL/glew.h>

namespace av {

Window::Window()
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_width(0)
    , m_height(0)
    , m_savedPosX(0)
    , m_savedPosY(0)
    , m_savedWidth(0)
    , m_savedHeight(0)
    , m_fullscreen(false)
    , m_minimized(false)
    , m_borderless(true) // Set to true for borderless draggable window
{
}

Window::~Window()
{
    shutdown();
}

bool Window::initialize(int width, int height, const std::string& title)
{
    std::cout << "Initializing window..." << std::endl;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "SDL initialized successfully" << std::endl;
    
    // Force OpenGL to use compatibility profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // Use OpenGL 2.1 for maximum compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Explicitly request a hardware accelerated renderer
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    std::cout << "OpenGL attributes set for maximum compatibility" << std::endl;
    
    // Create window with OpenGL context
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (m_borderless) {
        flags |= SDL_WINDOW_BORDERLESS;
    }
    
    std::cout << "Creating window with dimensions: " << width << "x" << height << std::endl;
    
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        flags
    );
    
    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "Window created successfully" << std::endl;
    
    // Create OpenGL context
    std::cout << "Creating OpenGL context..." << std::endl;
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "OpenGL context created successfully" << std::endl;
    
    // Make the OpenGL context current
    SDL_GL_MakeCurrent(m_window, m_glContext);
    std::cout << "OpenGL context made current" << std::endl;
    
    // Initialize GLEW
    std::cout << "Initializing GLEW..." << std::endl;
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
        return false;
    }
    
    std::cout << "GLEW initialized successfully" << std::endl;
    
    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Enable VSync
    SDL_GL_SetSwapInterval(1);
    std::cout << "VSync enabled" << std::endl;
    
    // Force an immediate buffer swap to check if rendering works
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Bright red
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(m_window);
    std::cout << "Initial red screen displayed" << std::endl;
    
    // Store window dimensions
    m_width = width;
    m_height = height;
    
    // Store original dimensions for restoring from fullscreen
    m_savedWidth = width;
    m_savedHeight = height;
    
    std::cout << "Window initialized successfully" << std::endl;
    return true;
}

void Window::shutdown()
{
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
}

void Window::swapBuffers()
{
    SDL_GL_SwapWindow(m_window);
}

void Window::toggleFullscreen()
{
    m_fullscreen = !m_fullscreen;
    
    if (m_fullscreen) {
        // Save current position and size before going fullscreen
        SDL_GetWindowPosition(m_window, &m_savedPosX, &m_savedPosY);
        SDL_GetWindowSize(m_window, &m_savedWidth, &m_savedHeight);
        
        // Switch to fullscreen
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        
        // Get the new dimensions
        SDL_GetWindowSize(m_window, &m_width, &m_height);
    } else {
        // Return to windowed mode
        SDL_SetWindowFullscreen(m_window, 0);
        
        // Restore position and size
        SDL_SetWindowSize(m_window, m_savedWidth, m_savedHeight);
        SDL_SetWindowPosition(m_window, m_savedPosX, m_savedPosY);
        
        m_width = m_savedWidth;
        m_height = m_savedHeight;
    }
}

void Window::setWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(m_window, x, y);
}

void Window::setWindowSize(int width, int height)
{
    SDL_SetWindowSize(m_window, width, height);
    m_width = width;
    m_height = height;
}

SDL_Window* Window::getSDLWindow() const
{
    return m_window;
}

bool Window::isBorderless() const
{
    return m_borderless;
}

} // namespace av 