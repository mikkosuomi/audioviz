#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <string>

namespace av {

/**
 * Structure to hold audio analysis results
 */
struct AudioData {
    float energy;          // Overall energy level (0.0-1.0)
    float bass;            // Bass level (0.0-1.0)
    float mid;             // Mid-range level (0.0-1.0)
    float treble;          // Treble level (0.0-1.0)
    float transient;       // Transient detection (sudden changes)
    std::vector<float> spectrum;    // Full frequency spectrum
    std::vector<float> waveform;    // Time-domain waveform
};

/**
 * Handles audio capture and analysis
 */
class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();

    // Initialize audio capture
    bool initialize(int sampleRate = 44100, int frameSize = 1024);
    
    // Shutdown and cleanup
    void shutdown();
    
    // Process audio and update analysis
    void update();
    
    // Get the latest audio analysis results
    const AudioData& getAudioData() const { return m_currentAudioData; }
    
    // Check if audio is being processed
    bool isAudioAvailable() const { return m_audioAvailable; }

private:
    // Implementation-specific data structure
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    // Analysis parameters
    int m_sampleRate;
    int m_frameSize;
    int m_historySize;
    
    // Audio analysis results
    AudioData m_currentAudioData;
    std::deque<AudioData> m_audioHistory;
    
    // Status flag
    bool m_audioAvailable;
    
    // Frequency band definitions
    float m_bassFrequencyLimit;    // Upper limit for bass (e.g., 250Hz)
    float m_midFrequencyLimit;     // Upper limit for mid (e.g., 2000Hz)
    float m_maxFrequency;          // Maximum frequency to analyze
    
    // Helper method to generate test audio data
    void generateTestData();
};

} // namespace av 