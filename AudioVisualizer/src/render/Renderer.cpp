#include "Renderer.h"
#include "Window.h"
#include "ShaderManager.h"
#include "ParticleSystem.h"
#include <iostream>
#include <GL/glew.h>
#include <cmath>
#include <SDL.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace av {

// Add this helper function at the top of the file, below namespace av {
void checkGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << operation << ": ";
        switch (error) {
            case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW: std::cerr << "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW: std::cerr << "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            default: std::cerr << "Unknown error: " << error; break;
        }
        std::cerr << std::endl;
    }
}

// Implementation of Color::fromHSV
Color Color::fromHSV(float h, float s, float v, float a) {
    float r, g, b;
    
    // Ensure h is in [0,1]
    h = std::fmod(h, 1.0f);
    if (h < 0.0f) h += 1.0f;
    
    // Convert to RGB
    int i = static_cast<int>(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = g = b = 0.0f; break;
    }
    
    return Color(r, g, b, a);
}

Renderer::Renderer(Window* window)
    : m_window(window)
    , m_particleSystem(nullptr)
    , m_shaderManager(nullptr)
    , m_mainFramebuffer(0)
    , m_effectFramebuffer(0)
    , m_colorTexture(0)
    , m_depthBuffer(0)
    , m_initialized(false)
    , m_width(0)
    , m_height(0)
{
}

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::initialize()
{
    std::cout << "Initializing renderer..." << std::endl;
    
    if (!m_window) {
        std::cerr << "Window is null" << std::endl;
        return false;
    }
    
    m_width = m_window->getWidth();
    m_height = m_window->getHeight();
    
    // Initialize OpenGL (basic setup)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, m_width, m_height);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize framebuffers for effects
    if (!initializeFramebuffers()) {
        std::cerr << "Failed to initialize framebuffers" << std::endl;
        return false;
    }
    
    // In a real implementation, you would initialize the shader manager here
    m_shaderManager = std::make_unique<ShaderManager>();
    
    // In a real implementation, you would initialize the particle system here
    m_particleSystem = std::make_unique<ParticleSystem>();
    
    m_initialized = true;
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    // Delete framebuffers
    if (m_mainFramebuffer != 0) {
        glDeleteFramebuffers(1, &m_mainFramebuffer);
        m_mainFramebuffer = 0;
    }
    
    if (m_effectFramebuffer != 0) {
        glDeleteFramebuffers(1, &m_effectFramebuffer);
        m_effectFramebuffer = 0;
    }
    
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    
    if (m_depthBuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthBuffer);
        m_depthBuffer = 0;
    }
    
    m_particleSystem.reset();
    m_shaderManager.reset();
    
    m_initialized = false;
    std::cout << "Renderer shutdown" << std::endl;
}

void Renderer::beginFrame()
{
    if (!m_initialized) {
        std::cerr << "Renderer not initialized in beginFrame" << std::endl;
        return;
    }
    
    std::cout << "Beginning frame - setting up rendering" << std::endl;

    // Bind our main framebuffer for normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
    checkGLError("binding main framebuffer");
    
    // Clear the framebuffer with a black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGLError("clearing main framebuffer");
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_width, m_height, 0, -1, 1);
    checkGLError("setting projection");
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    checkGLError("setting modelview");
}

void Renderer::endFrame()
{
    if (!m_initialized) {
        std::cerr << "Renderer not initialized in endFrame" << std::endl;
        return;
    }
    
    // Unbind any framebuffers - return to default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLError("binding default framebuffer");
    
    // Reset matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_width, m_height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render the framebuffer texture to the screen
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(m_width, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(m_width, m_height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, m_height);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    // Log that we're rendering
    std::cout << "Frame rendered to screen from framebuffer texture: " << m_colorTexture << std::endl;
    
    // Swap buffers to display what we just drew
    m_window->swapBuffers();
}

bool Renderer::initializeFramebuffers()
{
    std::cout << "Initializing framebuffers with dimensions: " << m_width << "x" << m_height << std::endl;
    
    // Create main framebuffer
    glGenFramebuffers(1, &m_mainFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
    
    checkGLError("generate framebuffer");
    std::cout << "Main framebuffer ID: " << m_mainFramebuffer << std::endl;
    
    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    
    checkGLError("creating color texture");
    std::cout << "Color texture ID: " << m_colorTexture << std::endl;
    
    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    
    checkGLError("creating depth buffer");
    std::cout << "Depth buffer ID: " << m_depthBuffer << std::endl;
    
    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete: " << status << std::endl;
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cerr << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
                break;
            default:
                std::cerr << "Unknown framebuffer status error" << std::endl;
        }
        return false;
    }
    
    std::cout << "Framebuffer complete" << std::endl;
    
    // Create effect framebuffer (for post-processing)
    glGenFramebuffers(1, &m_effectFramebuffer);
    
    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}

// Drawing primitives (simplified for placeholder)
void Renderer::drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness)
{
    glLineWidth(thickness);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void Renderer::drawCircle(float x, float y, float radius, const Color& color, float thickness)
{
    glLineWidth(thickness);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 36; ++i) {
        float angle = 2.0f * M_PI * i / 36.0f;
        glVertex2f(x + std::cos(angle) * radius, y + std::sin(angle) * radius);
    }
    glEnd();
}

void Renderer::drawFilledCircle(float x, float y, float radius, const Color& color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center
    for (int i = 0; i <= 36; ++i) {
        float angle = 2.0f * M_PI * i / 36.0f;
        glVertex2f(x + std::cos(angle) * radius, y + std::sin(angle) * radius);
    }
    glEnd();
}

void Renderer::drawRect(float x, float y, float width, float height, const Color& color, float thickness)
{
    glLineWidth(thickness);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Renderer::drawFilledRect(float x, float y, float width, float height, const Color& color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Renderer::drawPolygon(const float* points, int count, const Color& color, float thickness)
{
    if (count < 2) {
        return;
    }
    
    glLineWidth(thickness);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < count; i += 2) {
        glVertex2f(points[i], points[i + 1]);
    }
    glEnd();
}

void Renderer::drawFilledPolygon(const float* points, int count, const Color& color)
{
    if (count < 6) {  // Need at least 3 points (6 values) for a triangle
        return;
    }
    
    glColor4f(color.r, color.g, color.b, color.a);
    
    glBegin(GL_POLYGON);
    for (int i = 0; i < count; i += 2) {
        glVertex2f(points[i], points[i + 1]);
    }
    glEnd();
}

void Renderer::drawWaveform(const float* samples, int count, float x, float y, float width, float height, const Color& color)
{
    if (count < 2) {
        return;
    }
    
    // Use thicker lines for better visibility
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(8.0f); // Increased from 5.0f
    
    // Amplify the waveform by multiplying sample values
    const float amplifyFactor = 4.0f; // Increased from 2.5f
    
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < count; ++i) {
        float xPos = x + i * width / (count - 1);
        
        // Apply amplification to make waveform more visible
        float amplifiedSample = samples[i] * amplifyFactor;
        // Clamp to avoid drawing outside the bounds
        amplifiedSample = std::clamp(amplifiedSample, -1.0f, 1.0f);
        
        float yPos = y + height / 2 + amplifiedSample * height / 2;
        glVertex2f(xPos, yPos);
    }
    glEnd();
    
    // Debug output to see if waveform data is being received
    static int frameCount = 0;
    if (frameCount++ % 120 == 0) {
        float maxValue = 0.0f;
        for (int i = 0; i < count; i++) {
            maxValue = std::max(maxValue, std::abs(samples[i]));
        }
        std::cout << "Renderer drawing waveform with " << count << " samples, max amplitude: " 
                  << maxValue << ", amplified to: " << std::min(maxValue * amplifyFactor, 1.0f) << std::endl;
    }
}

void Renderer::drawSpectrum(const float* spectrum, int count, float x, float y, float width, float height, const Color& color)
{
    if (count < 2) {
        return;
    }
    
    glColor4f(color.r, color.g, color.b, color.a);
    
    const float barWidth = width / count;
    
    for (int i = 0; i < count; ++i) {
        float barHeight = spectrum[i] * height;
        float xPos = x + i * barWidth;
        float yPos = y + height - barHeight;  // Draw from bottom up
        
        drawFilledRect(xPos, yPos, barWidth * 0.9f, barHeight, color);
    }
}

void Renderer::drawParticle(float x, float y, float size, const Color& color, int shapeType)
{
    switch (shapeType % 6) {
        case 0:  // Circle
            drawFilledCircle(x, y, size, color);
            break;
        
        case 1: {  // Square
            drawFilledRect(x - size, y - size, size * 2, size * 2, color);
            break;
        }
        
        case 2: {  // Triangle
            float points[] = {
                x, y - size,
                x + size, y + size,
                x - size, y + size
            };
            drawFilledPolygon(points, 6, color);
            break;
        }
        
        case 3: {  // Star
            const int numPoints = 5;
            const float innerRadius = size * 0.4f;
            const float outerRadius = size;
            
            float points[numPoints * 4];
            for (int i = 0; i < numPoints; ++i) {
                float angle1 = M_PI / 2 + i * 2 * M_PI / numPoints;
                float angle2 = angle1 + M_PI / numPoints;
                
                points[i * 4] = x + std::cos(angle1) * outerRadius;
                points[i * 4 + 1] = y + std::sin(angle1) * outerRadius;
                points[i * 4 + 2] = x + std::cos(angle2) * innerRadius;
                points[i * 4 + 3] = y + std::sin(angle2) * innerRadius;
            }
            
            glColor4f(color.r, color.g, color.b, color.a);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y);  // Center
            for (int i = 0; i < numPoints * 2; ++i) {
                int idx = (i % (numPoints * 2)) * 2;
                glVertex2f(points[idx], points[idx + 1]);
            }
            glVertex2f(points[0], points[1]); // Close the fan
            glEnd();
            break;
        }
        
        case 4: {  // Diamond
            float points[] = {
                x, y - size,
                x + size, y,
                x, y + size,
                x - size, y
            };
            drawFilledPolygon(points, 8, color);
            break;
        }
        
        case 5: {  // Cross
            float thickness = size * 0.3f;
            
            // Vertical bar
            drawFilledRect(x - thickness / 2, y - size, thickness, size * 2, color);
            
            // Horizontal bar
            drawFilledRect(x - size, y - thickness / 2, size * 2, thickness, color);
            break;
        }
    }
}

// Special effects (simplified placeholders)
void Renderer::applyBlur(float strength)
{
    // In a real implementation, this would use shaders for a blur effect
    // For the placeholder, just do nothing
    std::cout << "Applying blur with strength " << strength << std::endl;
}

void Renderer::applyColorShift(const Color& color)
{
    // In a real implementation, this would use shaders for a color shift
    // For the placeholder, just do nothing
    std::cout << "Applying color shift " << color.r << "," << color.g << "," << color.b << std::endl;
}

void Renderer::applyKaleidoscope(int segments, float angle)
{
    // In a real implementation, this would use shaders for a kaleidoscope effect
    // For the placeholder, just do nothing
    std::cout << "Applying kaleidoscope with " << segments << " segments at angle " << angle << std::endl;
}

// Add the resize method implementation just before the drawing methods begin
void Renderer::resize(int width, int height)
{
    if (!m_initialized) {
        return;
    }
    
    if (width == m_width && height == m_height) {
        return; // No change needed
    }
    
    std::cout << "Resizing renderer from " << m_width << "x" << m_height << " to " << width << "x" << height << std::endl;
    
    m_width = width;
    m_height = height;
    
    // Update viewport
    glViewport(0, 0, width, height);
    
    // Clean up existing framebuffer attachments
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
    }
    
    if (m_depthBuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthBuffer);
    }
    
    // Recreate texture with new size
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Bind to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    
    // Recreate depth buffer
    glGenRenderbuffers(1, &m_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    
    // Check for framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete after resize: " << status << std::endl;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    checkGLError("resize renderer");
}

} // namespace av 