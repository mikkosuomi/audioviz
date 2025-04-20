#pragma once

#include <memory>
#include <string>
#include <array>

namespace av {

// Forward declarations
class Window;
class ParticleSystem;
class ShaderManager;

/**
 * Simple color structure
 */
struct Color {
    float r, g, b, a;
    
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    // Create from hex color (0xRRGGBB)
    static Color fromHex(unsigned int hex, float alpha = 1.0f) {
        return Color(
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f,
            alpha
        );
    }
    
    // Create from HSV
    static Color fromHSV(float h, float s, float v, float a = 1.0f);
};

/**
 * Handles all rendering operations
 */
class Renderer {
public:
    Renderer(Window* window);
    ~Renderer();

    // Initialize the renderer
    bool initialize();
    
    // Shutdown and cleanup
    void shutdown();
    
    // Frame management
    void beginFrame();
    void endFrame();
    
    // Get dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Handle window size changes
    void resize(int width, int height);
    
    // Basic drawing primitives
    void drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f);
    void drawCircle(float x, float y, float radius, const Color& color, float thickness = 1.0f);
    void drawFilledCircle(float x, float y, float radius, const Color& color);
    void drawRect(float x, float y, float width, float height, const Color& color, float thickness = 1.0f);
    void drawFilledRect(float x, float y, float width, float height, const Color& color);
    void drawPolygon(const float* points, int count, const Color& color, float thickness = 1.0f);
    void drawFilledPolygon(const float* points, int count, const Color& color);
    
    // Visualization-specific drawing
    void drawWaveform(const float* samples, int count, float x, float y, float width, float height, const Color& color);
    void drawSpectrum(const float* spectrum, int count, float x, float y, float width, float height, const Color& color);
    void drawParticle(float x, float y, float size, const Color& color, int shapeType = 0);
    
    // Special effects
    void applyBlur(float strength);
    void applyColorShift(const Color& color);
    void applyKaleidoscope(int segments, float angle);
    
    // Advanced rendering features
    ParticleSystem* getParticleSystem() { return m_particleSystem.get(); }
    ShaderManager* getShaderManager() { return m_shaderManager.get(); }

private:
    Window* m_window;
    
    // Rendering subsystems
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<ShaderManager> m_shaderManager;
    
    // Render targets for effects
    unsigned int m_mainFramebuffer;
    unsigned int m_effectFramebuffer;
    unsigned int m_colorTexture;
    unsigned int m_depthBuffer;
    
    // Rendering state
    bool m_initialized;
    int m_width;
    int m_height;
    
    // Initialize framebuffers for effects
    bool initializeFramebuffers();
};

} // namespace av 