#include "AudioData.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace av {

AudioData::AudioData()
    : m_sampleRate(44100)
    , m_channels(2)
    , m_bassLevel(0.0f)
    , m_midLevel(0.0f)
    , m_trebleLevel(0.0f)
    , m_volumeLevel(0.0f)
{
    // Initialize with default sizes
    waveformData.resize(1024, 0.0f);
    frequencyData.resize(512, 0.0f);
    previousFrequencyData.resize(512, 0.0f);
    
    // Generate Hann window for FFT processing
    generateHannWindow(1024);
}

AudioData::~AudioData() {
    // No dynamic memory to clean up
}

void AudioData::update(const float* samples, size_t sampleCount, int channels, int sampleRate) {
    m_sampleRate = sampleRate;
    m_channels = channels;
    
    // Resize waveform buffer if needed
    if (waveformData.size() != sampleCount) {
        waveformData.resize(sampleCount, 0.0f);
        frequencyData.resize(sampleCount / 2, 0.0f);
        previousFrequencyData.resize(sampleCount / 2, 0.0f);
        
        // Window size needs to match the waveform size
        generateHannWindow(sampleCount);
    }
    
    // Copy samples to waveform data, averaging multiple channels if needed
    if (channels == 1) {
        // Mono: direct copy
        std::copy(samples, samples + sampleCount, waveformData.begin());
    } 
    else {
        // Stereo or more: average channels
        for (size_t i = 0; i < sampleCount; i++) {
            float sum = 0.0f;
            for (int c = 0; c < channels; c++) {
                sum += samples[i * channels + c];
            }
            waveformData[i] = sum / static_cast<float>(channels);
        }
    }
    
    // Calculate overall volume level
    float sum = 0.0f;
    for (const float& sample : waveformData) {
        sum += std::abs(sample);
    }
    m_volumeLevel = sum / static_cast<float>(waveformData.size());
    
    // Process FFT to get frequency data
    processFFT();
    
    // Calculate frequency band levels
    m_bassLevel = 0.0f;
    m_midLevel = 0.0f;
    m_trebleLevel = 0.0f;
    
    // Simple band calculation (can be refined based on actual frequency ranges)
    const size_t bassLimit = frequencyData.size() / 8;     // Lower 1/8th for bass
    const size_t midLimit = frequencyData.size() / 2;      // Next 3/8ths for mids
    
    // Sum up values in each frequency band
    for (size_t i = 0; i < frequencyData.size(); i++) {
        if (i < bassLimit) {
            m_bassLevel += frequencyData[i];
        } 
        else if (i < midLimit) {
            m_midLevel += frequencyData[i];
        } 
        else {
            m_trebleLevel += frequencyData[i];
        }
    }
    
    // Normalize the levels
    if (bassLimit > 0) {
        m_bassLevel /= static_cast<float>(bassLimit);
    }
    
    if (midLimit - bassLimit > 0) {
        m_midLevel /= static_cast<float>(midLimit - bassLimit);
    }
    
    if (frequencyData.size() - midLimit > 0) {
        m_trebleLevel /= static_cast<float>(frequencyData.size() - midLimit);
    }
}

void AudioData::processFFT() {
    // This is a simplified placeholder FFT implementation
    // In a real implementation, you would use a proper FFT library like FFTW or DSP libraries
    
    // For now, we'll generate simulated frequency data from the waveform
    // Note: This does not perform a real FFT, but gives a basic frequency response
    
    const size_t freqSize = waveformData.size() / 2;
    std::vector<float> tempFreq(freqSize, 0.0f);
    
    // Apply window function to reduce spectral leakage
    std::vector<float> windowedData(waveformData.size());
    for (size_t i = 0; i < waveformData.size(); i++) {
        windowedData[i] = waveformData[i] * m_window[i];
    }
    
    // Simulate frequency bands with simple filters
    for (size_t i = 0; i < freqSize; i++) {
        float amplitude = 0.0f;
        
        // Simple approximation - use different sample ranges for different frequency bands
        // Lower frequencies capture more samples, higher frequencies use fewer samples
        size_t range = waveformData.size() / (i + 1);
        range = std::min(range, waveformData.size() / 4);
        range = std::max(range, size_t(4));
        
        for (size_t j = 0; j < range; j++) {
            size_t index = (i * j) % waveformData.size();
            amplitude += std::abs(windowedData[index]);
        }
        
        tempFreq[i] = amplitude / static_cast<float>(range);
    }
    
    // Copy to the frequency data buffer
    std::swap(frequencyData, tempFreq);
    
    // Normalize and smooth the frequency data
    normalizeFrequencyData();
}

void AudioData::normalizeFrequencyData(float smoothingFactor) {
    // Find the maximum value for normalization
    float maxValue = *std::max_element(frequencyData.begin(), frequencyData.end());
    
    // Avoid division by zero
    if (maxValue <= 0.0001f) {
        maxValue = 0.0001f;
    }
    
    // Normalize and apply smoothing
    for (size_t i = 0; i < frequencyData.size(); i++) {
        // Normalize to 0.0-1.0 range
        float normalizedValue = frequencyData[i] / maxValue;
        
        // Apply smoothing with previous frame data
        if (i < previousFrequencyData.size()) {
            frequencyData[i] = previousFrequencyData[i] * smoothingFactor + 
                              normalizedValue * (1.0f - smoothingFactor);
        } else {
            frequencyData[i] = normalizedValue;
        }
    }
    
    // Store current frequency data for next frame smoothing
    previousFrequencyData = frequencyData;
}

void AudioData::generateHannWindow(size_t size) {
    m_window.resize(size);
    
    for (size_t i = 0; i < size; i++) {
        // Hann window function: 0.5 * (1 - cos(2π*n/(N-1)))
        m_window[i] = 0.5f * (1.0f - std::cos(2.0f * 3.14159f * static_cast<float>(i) / 
                                           static_cast<float>(size - 1)));
    }
}

float AudioData::getBassLevel() const {
    return m_bassLevel;
}

float AudioData::getMidLevel() const {
    return m_midLevel;
}

float AudioData::getTrebleLevel() const {
    return m_trebleLevel;
}

float AudioData::getVolumeLevel() const {
    return m_volumeLevel;
}

} // namespace av 