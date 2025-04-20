#include "FFTAnalyzer.h"
#include <cmath>
#include <algorithm>

namespace av {

FFTAnalyzer::FFTAnalyzer() 
    : m_sampleRate(0)
    , m_fftSize(0)
    , m_hopSize(0)
    , m_bufferPosition(0)
    , m_initialized(false)
{
}

FFTAnalyzer::~FFTAnalyzer() {
    shutdown();
}

bool FFTAnalyzer::initialize(int sampleRate, int fftSize, int hopSize) {
    if (m_initialized) {
        shutdown();
    }
    
    m_sampleRate = sampleRate;
    m_fftSize = fftSize;
    m_hopSize = hopSize;
    
    // Initialize buffers
    m_inputBuffer.resize(m_fftSize, 0.0f);
    m_fftBuffer.resize(m_fftSize, std::complex<float>(0.0f, 0.0f));
    m_frequencyData.resize(m_fftSize / 2 + 1, 0.0f);
    
    // Create Hann window for better frequency resolution
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; i++) {
        m_window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (m_fftSize - 1)));
    }
    
    m_bufferPosition = 0;
    m_initialized = true;
    
    return true;
}

void FFTAnalyzer::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_inputBuffer.clear();
    m_fftBuffer.clear();
    m_frequencyData.clear();
    m_window.clear();
    
    m_initialized = false;
}

void FFTAnalyzer::processAudioBuffer(const float* buffer, int bufferSize) {
    if (!m_initialized || !buffer) {
        return;
    }
    
    // Add new samples to the input buffer
    for (int i = 0; i < bufferSize; i++) {
        m_inputBuffer[m_bufferPosition] = buffer[i];
        m_bufferPosition++;
        
        // When we have collected enough samples, perform FFT
        if (m_bufferPosition >= m_hopSize) {
            performFFT();
            
            // Shift buffer by hop size to make room for new samples
            for (int j = 0; j < m_fftSize - m_hopSize; j++) {
                m_inputBuffer[j] = m_inputBuffer[j + m_hopSize];
            }
            
            // Reset position to account for shifted samples
            m_bufferPosition -= m_hopSize;
        }
    }
}

void FFTAnalyzer::performFFT() {
    // Apply window function to reduce spectral leakage
    applyWindow();
    
    // Simple DFT implementation (in a real implementation, use an FFT library)
    for (int k = 0; k < m_fftSize; k++) {
        m_fftBuffer[k] = std::complex<float>(0.0f, 0.0f);
        
        for (int n = 0; n < m_fftSize; n++) {
            float angle = -2.0f * M_PI * k * n / m_fftSize;
            std::complex<float> expTerm(cosf(angle), sinf(angle));
            m_fftBuffer[k] += m_inputBuffer[n] * expTerm;
        }
    }
    
    // Compute magnitude spectrum
    computeMagnitudes();
}

void FFTAnalyzer::applyWindow() {
    for (int i = 0; i < m_fftSize; i++) {
        m_inputBuffer[i] *= m_window[i];
    }
}

void FFTAnalyzer::computeMagnitudes() {
    // Compute magnitude spectrum (only need half the FFT result due to symmetry)
    float normalizationFactor = 1.0f / m_fftSize;
    
    for (int i = 0; i <= m_fftSize / 2; i++) {
        float real = m_fftBuffer[i].real();
        float imag = m_fftBuffer[i].imag();
        float magnitude = sqrtf(real * real + imag * imag) * normalizationFactor;
        
        // Convert to decibels with some scaling and lower limit
        float magnitudeDB = 20.0f * log10f(std::max(magnitude, 1e-6f));
        m_frequencyData[i] = std::max(-100.0f, magnitudeDB);
    }
}

const std::vector<float>& FFTAnalyzer::getFrequencyData() const {
    return m_frequencyData;
}

float FFTAnalyzer::getLowFrequencyMagnitude() const {
    if (!m_initialized) {
        return 0.0f;
    }
    
    // Calculate frequency band indices (20-250Hz)
    int lowIndex = static_cast<int>(20.0f * m_fftSize / m_sampleRate);
    int highIndex = static_cast<int>(250.0f * m_fftSize / m_sampleRate);
    
    // Ensure indices are within valid range
    lowIndex = std::max(0, lowIndex);
    highIndex = std::min(static_cast<int>(m_frequencyData.size() - 1), highIndex);
    
    // Calculate average magnitude in the band
    float sum = 0.0f;
    for (int i = lowIndex; i <= highIndex; i++) {
        sum += m_frequencyData[i];
    }
    
    return (highIndex >= lowIndex) ? sum / (highIndex - lowIndex + 1) : 0.0f;
}

float FFTAnalyzer::getMidFrequencyMagnitude() const {
    if (!m_initialized) {
        return 0.0f;
    }
    
    // Calculate frequency band indices (250-4000Hz)
    int lowIndex = static_cast<int>(250.0f * m_fftSize / m_sampleRate);
    int highIndex = static_cast<int>(4000.0f * m_fftSize / m_sampleRate);
    
    // Ensure indices are within valid range
    lowIndex = std::max(0, lowIndex);
    highIndex = std::min(static_cast<int>(m_frequencyData.size() - 1), highIndex);
    
    // Calculate average magnitude in the band
    float sum = 0.0f;
    for (int i = lowIndex; i <= highIndex; i++) {
        sum += m_frequencyData[i];
    }
    
    return (highIndex >= lowIndex) ? sum / (highIndex - lowIndex + 1) : 0.0f;
}

float FFTAnalyzer::getHighFrequencyMagnitude() const {
    if (!m_initialized) {
        return 0.0f;
    }
    
    // Calculate frequency band indices (4000-20000Hz)
    int lowIndex = static_cast<int>(4000.0f * m_fftSize / m_sampleRate);
    int highIndex = static_cast<int>(20000.0f * m_fftSize / m_sampleRate);
    
    // Ensure indices are within valid range
    lowIndex = std::max(0, lowIndex);
    highIndex = std::min(static_cast<int>(m_frequencyData.size() - 1), highIndex);
    
    // Calculate average magnitude in the band
    float sum = 0.0f;
    for (int i = lowIndex; i <= highIndex; i++) {
        sum += m_frequencyData[i];
    }
    
    return (highIndex >= lowIndex) ? sum / (highIndex - lowIndex + 1) : 0.0f;
}

} // namespace av 