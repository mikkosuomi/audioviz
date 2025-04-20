#pragma once

#include <vector>
#include <cstdint>

namespace av {

/**
 * @brief Class for storing and processing audio data for visualization
 * 
 * This class handles both raw waveform data and processed frequency data.
 * It provides methods for FFT processing and frequency band analysis.
 */
class AudioData {
public:
    AudioData();
    ~AudioData();

    /**
     * @brief Update audio data with new samples
     * 
     * @param samples Pointer to audio sample data
     * @param sampleCount Number of samples
     * @param channels Number of audio channels
     * @param sampleRate Sample rate in Hz
     */
    void update(const float* samples, size_t sampleCount, int channels, int sampleRate);

    /**
     * @brief Process audio data with FFT to generate frequency data
     */
    void processFFT();

    /**
     * @brief Normalize frequency data to 0.0-1.0 range
     * 
     * @param smoothingFactor Amount of smoothing between frames (0.0-1.0)
     */
    void normalizeFrequencyData(float smoothingFactor = 0.5f);

    // Getters for frequency bands
    float getBassLevel() const;
    float getMidLevel() const;
    float getTrebleLevel() const;
    float getVolumeLevel() const;

    // Public data for direct access in visualizations
    std::vector<float> waveformData;        // Raw audio waveform data
    std::vector<float> frequencyData;       // Processed frequency data
    std::vector<float> previousFrequencyData; // Previous frame's frequency data

private:
    /**
     * @brief Generate a Hann window for FFT processing
     * 
     * @param size Size of the window
     */
    void generateHannWindow(size_t size);

    std::vector<float> m_window;        // Window function for FFT
    int m_sampleRate;                  // Sample rate in Hz
    int m_channels;                    // Number of audio channels

    // Frequency band levels
    float m_bassLevel;                 // Bass level (low frequencies)
    float m_midLevel;                  // Mid level (mid frequencies)
    float m_trebleLevel;               // Treble level (high frequencies)
    float m_volumeLevel;               // Overall volume level
};

} // namespace av 