#include "visualizations/Bars3DVisualizer.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SDL.h>
#include <GL/glew.h>

namespace av {

Bars3DVisualizer::Bars3DVisualizer()
    : Visualization("3D Bars")
    , m_gridSize(16)  // 16x16 grid of bars
    , m_maxBarHeight(200.0f)
    , m_barWidth(0.8f)
    , m_spacing(1.2f)
    , m_rotationAngle(0.0f)
    , m_cameraHeight(50.0f)
    , m_lastUpdateTime(0.0f)
    , m_smoothingFactor(0.15f)
{
    std::cout << "Bars3DVisualizer created" << std::endl;
}

Bars3DVisualizer::~Bars3DVisualizer()
{
    cleanup();
}

bool Bars3DVisualizer::initialize(Renderer* renderer)
{
    // Initialize the grid of bars
    m_bars.clear();
    m_bars.reserve(m_gridSize * m_gridSize);
    
    // Calculate grid center for centering
    float centerOffset = (m_gridSize * m_spacing) / 2.0f;
    
    // Create grid of bars
    for (int z = 0; z < m_gridSize; z++) {
        for (int x = 0; x < m_gridSize; x++) {
            Bar bar;
            bar.height = 0.0f;
            bar.targetHeight = 0.0f;
            bar.x = x * m_spacing - centerOffset;
            bar.z = z * m_spacing - centerOffset;
            
            // Assign colors based on position in grid
            float distanceFromCenter = std::sqrt(
                std::pow((x - m_gridSize / 2.0f) / m_gridSize, 2) +
                std::pow((z - m_gridSize / 2.0f) / m_gridSize, 2)
            ) * 2.0f;
            distanceFromCenter = std::min(1.0f, distanceFromCenter);
            
            // Use polar coordinates for coloring
            float angle = std::atan2(z - m_gridSize / 2.0f, x - m_gridSize / 2.0f);
            angle = (angle + M_PI) / (2.0f * M_PI); // Normalize to 0-1
            bar.hue = angle;
            
            m_bars.push_back(bar);
        }
    }
    
    // Call parent implementation
    return Visualization::initialize(renderer);
}

void Bars3DVisualizer::cleanup()
{
    // No resources to clean up
}

void Bars3DVisualizer::onResize(int width, int height)
{
    // Nothing specific to do here
}

void Bars3DVisualizer::updateBars(const AudioData& audioData, float deltaTime)
{
    // Calculate rotation speed based on energy
    float rotationSpeed = 10.0f + audioData.energy * 30.0f;
    m_rotationAngle += rotationSpeed * deltaTime;
    
    // Wrap angle to 0-360
    while (m_rotationAngle > 360.0f) {
        m_rotationAngle -= 360.0f;
    }
    
    // Map spectrum frequencies to grid positions
    const size_t spectrumSize = audioData.spectrum.size();
    if (spectrumSize == 0) {
        return;
    }
    
    const int totalBars = m_bars.size();
    
    // Update each bar
    for (int i = 0; i < totalBars; i++) {
        // Map bar index to spectrum index using a logarithmic scale for better frequency distribution
        float spectrumIndex;
        
        // Calculate radial distance from center
        int x = i % m_gridSize;
        int z = i / m_gridSize;
        float centerX = m_gridSize / 2.0f;
        float centerZ = m_gridSize / 2.0f;
        float distanceFromCenter = std::sqrt(
            std::pow(x - centerX, 2) + 
            std::pow(z - centerZ, 2)
        ) / (m_gridSize / 2.0f);
        
        // Use distance to map frequency range - outer bars get higher frequencies
        spectrumIndex = distanceFromCenter * spectrumSize * 0.8f;
        
        // Clamp to valid range
        spectrumIndex = std::min(static_cast<float>(spectrumSize - 1), std::max(0.0f, spectrumIndex));
        
        // Get spectrum value
        float value = 0.0f;
        if (spectrumIndex < spectrumSize) {
            value = audioData.spectrum[static_cast<size_t>(spectrumIndex)];
        }
        
        // Apply amplification to make it more visible
        value = std::min(1.0f, value * 3.0f);
        
        // Set target height
        m_bars[i].targetHeight = value * m_maxBarHeight;
        
        // Smooth transition to target height
        float diff = m_bars[i].targetHeight - m_bars[i].height;
        m_bars[i].height += diff * m_smoothingFactor;
    }
    
    // Make the bars in the center respond to bass
    float bassImpact = audioData.bass * 2.0f;
    for (int i = 0; i < totalBars; i++) {
        int x = i % m_gridSize;
        int z = i / m_gridSize;
        float centerX = m_gridSize / 2.0f;
        float centerZ = m_gridSize / 2.0f;
        float distanceFromCenter = std::sqrt(
            std::pow(x - centerX, 2) + 
            std::pow(z - centerZ, 2)
        ) / (m_gridSize / 2.0f);
        
        // Apply bass to center bars
        if (distanceFromCenter < 0.3f) {
            float bassInfluence = (1.0f - distanceFromCenter / 0.3f) * bassImpact;
            m_bars[i].height += m_maxBarHeight * 0.2f * bassInfluence;
        }
    }
}

void Bars3DVisualizer::setup3DView(int width, int height)
{
    // Set up perspective projection without gluPerspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Manual perspective projection (replaces gluPerspective)
    float fov = 45.0f * 3.14159f / 180.0f; // Field of view in radians
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float zNear = 0.1f;
    float zFar = 1000.0f;
    float f = 1.0f / tan(fov / 2.0f);
    
    float perspective[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
        0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f
    };
    
    glMultMatrixf(perspective);
    
    // Set up camera view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Manual implementation of gluLookAt
    // Eye position: (0.0f, m_cameraHeight, m_cameraHeight * 0.8f)
    // Look at position: (0.0f, 0.0f, 0.0f)
    // Up vector: (0.0f, 1.0f, 0.0f)
    
    float eyeX = 0.0f;
    float eyeY = m_cameraHeight;
    float eyeZ = m_cameraHeight * 0.8f;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;
    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;
    
    // Calculate forward vector (f)
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;
    
    // Normalize f
    float len = sqrt(fx*fx + fy*fy + fz*fz);
    fx /= len;
    fy /= len;
    fz /= len;
    
    // Calculate side vector (s = f × up)
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    
    // Normalize s
    len = sqrt(sx*sx + sy*sy + sz*sz);
    sx /= len;
    sy /= len;
    sz /= len;
    
    // Calculate up vector (u = s × f)
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;
    
    // Create view matrix
    float viewMatrix[16] = {
        sx, ux, -fx, 0.0f,
        sy, uy, -fy, 0.0f,
        sz, uz, -fz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    glMultMatrixf(viewMatrix);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
    
    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    
    // Clear depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Bars3DVisualizer::draw3DBars(Renderer* renderer)
{
    // Rotate the entire scene
    glRotatef(m_rotationAngle, 0.0f, 1.0f, 0.0f);
    
    // Draw each bar
    for (const auto& bar : m_bars) {
        // Calculate bar position
        float x = bar.x;
        float z = bar.z;
        
        // Skip bars with no height
        if (bar.height < 0.1f) {
            continue;
        }
        
        // Calculate color based on height and hue
        float hue = bar.hue;
        float saturation = 0.8f;
        float value = 0.7f + 0.3f * (bar.height / m_maxBarHeight);
        
        Color color = Color::fromHSV(hue, saturation, value);
        Color topColor = Color::fromHSV(hue, saturation * 0.8f, std::min(1.0f, value * 1.3f));
        
        // Draw the bar as a 3D box
        float halfWidth = m_barWidth / 2.0f;
        
        // Draw top face (different color)
        glBegin(GL_QUADS);
        glColor4f(topColor.r, topColor.g, topColor.b, topColor.a);
        glVertex3f(x - halfWidth, bar.height, z - halfWidth);
        glVertex3f(x + halfWidth, bar.height, z - halfWidth);
        glVertex3f(x + halfWidth, bar.height, z + halfWidth);
        glVertex3f(x - halfWidth, bar.height, z + halfWidth);
        glEnd();
        
        // Draw sides with main color
        glBegin(GL_QUADS);
        glColor4f(color.r, color.g, color.b, color.a);
        
        // Front face
        glVertex3f(x - halfWidth, 0.0f, z - halfWidth);
        glVertex3f(x + halfWidth, 0.0f, z - halfWidth);
        glVertex3f(x + halfWidth, bar.height, z - halfWidth);
        glVertex3f(x - halfWidth, bar.height, z - halfWidth);
        
        // Right face
        glVertex3f(x + halfWidth, 0.0f, z - halfWidth);
        glVertex3f(x + halfWidth, 0.0f, z + halfWidth);
        glVertex3f(x + halfWidth, bar.height, z + halfWidth);
        glVertex3f(x + halfWidth, bar.height, z - halfWidth);
        
        // Back face
        glVertex3f(x + halfWidth, 0.0f, z + halfWidth);
        glVertex3f(x - halfWidth, 0.0f, z + halfWidth);
        glVertex3f(x - halfWidth, bar.height, z + halfWidth);
        glVertex3f(x + halfWidth, bar.height, z + halfWidth);
        
        // Left face
        glVertex3f(x - halfWidth, 0.0f, z + halfWidth);
        glVertex3f(x - halfWidth, 0.0f, z - halfWidth);
        glVertex3f(x - halfWidth, bar.height, z - halfWidth);
        glVertex3f(x - halfWidth, bar.height, z + halfWidth);
        glEnd();
    }
}

void Bars3DVisualizer::reset3DView()
{
    // Disable depth testing
    glDisable(GL_DEPTH_TEST);
    
    // Reset projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Bars3DVisualizer::render(Renderer* renderer, const AudioData& audioData)
{
    // Get window dimensions
    int width = renderer->getWidth();
    int height = renderer->getHeight();
    
    // Calculate delta time for animation
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - m_lastUpdateTime) / 1000.0f;
    m_lastUpdateTime = currentTime;
    
    // Update bars based on audio data
    updateBars(audioData, deltaTime);
    
    // Clear the background with a dark blue color
    glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up 3D view
    setup3DView(width, height);
    
    // Draw the 3D bars
    draw3DBars(renderer);
    
    // Reset the view back to 2D
    reset3DView();
    
    // Draw a spectrum analyzer at the bottom of the screen for reference
    int spectrumHeight = 50;
    int spectrumY = height - spectrumHeight - 10;
    
    // Draw spectrum if available
    if (!audioData.spectrum.empty()) {
        int barCount = std::min(64, static_cast<int>(audioData.spectrum.size()));
        float barWidth = static_cast<float>(width - 20) / barCount;
        
        for (int i = 0; i < barCount; i++) {
            float value = audioData.spectrum[i * audioData.spectrum.size() / barCount];
            value = std::min(1.0f, value * 3.0f); // Amplify for visibility
            
            float barHeight = value * spectrumHeight;
            float x = 10 + i * barWidth;
            float y = spectrumY + spectrumHeight - barHeight;
            
            // Color based on frequency
            float hue = static_cast<float>(i) / barCount;
            Color color = Color::fromHSV(hue, 0.8f, 0.9f);
            
            renderer->drawFilledRect(x, y, barWidth - 1, barHeight, color);
        }
    }
    
    // Draw energy indicators
    int indicatorSize = 30;
    int indicatorY = height - indicatorSize - 10;
    int spacing = 40;
    
    // Bass indicator (red)
    Color bassColor = Color::fromHSV(0.0f, 0.9f, 0.9f * audioData.bass);
    renderer->drawFilledCircle(width - spacing * 3, indicatorY, indicatorSize * audioData.bass, bassColor);
    
    // Mid indicator (green)
    Color midColor = Color::fromHSV(0.33f, 0.9f, 0.9f * audioData.mid);
    renderer->drawFilledCircle(width - spacing * 2, indicatorY, indicatorSize * audioData.mid, midColor);
    
    // Treble indicator (blue)
    Color trebleColor = Color::fromHSV(0.66f, 0.9f, 0.9f * audioData.treble);
    renderer->drawFilledCircle(width - spacing, indicatorY, indicatorSize * audioData.treble, trebleColor);
}

std::string Bars3DVisualizer::getDescription() const
{
    return "3D bars that react to frequency spectrum";
}

} // namespace av 