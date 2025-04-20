#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace av {

// Forward declarations
class Engine;
class AudioProcessor;
class Renderer;
struct AudioData;

/**
 * Handles scripting integration (Lua)
 */
class ScriptEngine {
public:
    ScriptEngine(Engine* engine);
    ~ScriptEngine();

    // Initialize the script engine
    bool initialize();
    
    // Shutdown and cleanup
    void shutdown();
    
    // Load a visualization script
    bool loadScript(const std::string& scriptPath);
    
    // Script lifecycle functions
    bool onInit();
    bool onUpdate(float deltaTime, const AudioData& audioData);
    bool onRender(Renderer* renderer);
    bool onShutdown();
    
    // Script interaction
    bool callFunction(const std::string& name);
    
    template<typename... Args>
    bool callFunction(const std::string& name, Args&&... args);
    
    // Status
    bool isScriptLoaded() const { return m_scriptLoaded; }
    const std::string& getScriptPath() const { return m_scriptPath; }
    const std::string& getLastError() const { return m_lastError; }

private:
    // Implementation-specific data structure
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    // Access to engine subsystems
    Engine* m_engine;
    
    // Script state
    bool m_scriptLoaded;
    std::string m_scriptPath;
    std::string m_lastError;
    
    // Register C++ functions to make them available to Lua
    void registerFunctions();
    
    // Helper methods to pass data to/from Lua
    void pushAudioData(const AudioData& audioData);
};

} // namespace av 