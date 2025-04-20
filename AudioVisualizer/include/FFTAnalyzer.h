#pragma once

#include <vector>
#include <complex>
#include <memory>

namespace av {

/**
 * Fast Fourier Transform audio analyzer
 * Processes audio data to extract frequency information
 */
class FFTAnalyzer {
public:
    FFTAnalyzer();
    ~FFTAnalyzer();
    
    // Initialize with specified parameters
    bool initialize(int sampleRate, int fftSize, int hopSize);
    void shutdown();
    
    // Process a new buffer of audio samples
    void processAudioBuffer(const float* buffer, int bufferSize);
    
    // Get the frequency data after FFT analysis
    const std::vector<float>& getFrequencyData() const;
    
    // Get specific frequency bands
    float getLowFrequencyMagnitude() const;  // Bass (20-250Hz)
    float getMidFrequencyMagnitude() const;  // Mids (250-4000Hz)
    float getHighFrequencyMagnitude() const; // Highs (4000-20000Hz)
    
    // Get properties
    int getSampleRate() const { return m_sampleRate; }
    int getFFTSize() const { return m_fftSize; }
    int getHopSize() const { return m_hopSize; }
    
private:
    // Helper methods for FFT computation
    void performFFT();
    void applyWindow();
    void computeMagnitudes();
    
    // FFT parameters
    int m_sampleRate;
    int m_fftSize;
    int m_hopSize;
    
    // Audio buffers
    std::vector<float> m_inputBuffer;
    std::vector<std::complex<float>> m_fftBuffer;
    std::vector<float> m_frequencyData;
    std::vector<float> m_window;
    
    // State tracking
    int m_bufferPosition;
    bool m_initialized;
};

} // namespace av 