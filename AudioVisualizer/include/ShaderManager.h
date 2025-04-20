#pragma once

#include "Renderer.h"
#include <string>
#include <unordered_map>

namespace av {

/**
 * Manages OpenGL shader programs
 */
class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();
    
    // Initialize and cleanup
    bool initialize();
    void shutdown();
    
    // Load and compile a shader program from source files
    unsigned int loadShader(const std::string& name, const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
    
    // Use a shader program
    void useShader(const std::string& name);
    
    // Set uniform values
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float x, float y);
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniform(const std::string& name, const Color& color);
    
    // Get shader program IDs
    unsigned int getShaderProgram(const std::string& name) const;
    unsigned int getCurrentShaderProgram() const { return m_currentShader; }
    
private:
    // Utility functions for shader compilation
    unsigned int compileShader(const std::string& source, unsigned int type);
    unsigned int createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader);
    std::string loadShaderFile(const std::string& filename);
    
    // Shader storage
    std::unordered_map<std::string, unsigned int> m_shaders;
    unsigned int m_currentShader;
};

} // namespace av 