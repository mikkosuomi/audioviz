#include "AudioProcessor.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// SDL for audio capture
#include <SDL.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Fix potential std::min/max conflicts with Windows macros
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

// Windows-specific headers for audio loopback capture
#ifdef _WIN32
#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>
#endif

namespace av {

// Implementation-specific data
struct AudioProcessor::Impl {
    SDL_AudioDeviceID deviceId;
    SDL_AudioSpec wantedSpec;
    SDL_AudioSpec obtainedSpec;
    std::vector<float> buffer;
    
#ifdef _WIN32
    // WASAPI-specific members for loopback capture
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pWaveFormat = nullptr;
    HANDLE captureEvent = nullptr;
    bool wasapiInitialized = false;
    UINT32 bufferFrameCount = 0;
#endif
};

AudioProcessor::AudioProcessor()
    : m_impl(new Impl())
    , m_sampleRate(44100)
    , m_frameSize(1024)
    , m_historySize(60)
    , m_audioAvailable(false)
    , m_bassFrequencyLimit(250.0f)
    , m_midFrequencyLimit(2000.0f)
    , m_maxFrequency(20000.0f)
{
    // Initialize audio data
    m_currentAudioData.energy = 0.0f;
    m_currentAudioData.bass = 0.0f;
    m_currentAudioData.mid = 0.0f;
    m_currentAudioData.treble = 0.0f;
    m_currentAudioData.transient = 0.0f;
    m_currentAudioData.spectrum.resize(m_frameSize / 2 + 1, 0.0f);
    m_currentAudioData.waveform.resize(m_frameSize, 0.0f);
}

AudioProcessor::~AudioProcessor()
{
    shutdown();
}

bool AudioProcessor::initialize(int sampleRate, int frameSize)
{
    std::cout << "Initializing audio processor..." << std::endl;
    
    m_sampleRate = sampleRate;
    m_frameSize = frameSize;
    
    // Initialize SDL Audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL audio could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Resize buffers
    m_impl->buffer.resize(m_frameSize, 0.0f);
    m_currentAudioData.spectrum.resize(m_frameSize / 2 + 1, 0.0f);
    m_currentAudioData.waveform.resize(m_frameSize, 0.0f);
    
#ifdef _WIN32
    // Initialize WASAPI for loopback capture
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library" << std::endl;
        return false;
    }
    
    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), 
                          reinterpret_cast<void**>(&m_impl->pEnumerator));
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator" << std::endl;
        return false;
    }
    
    // Get default audio render device (speakers/headphones)
    hr = m_impl->pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_impl->pDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint" << std::endl;
        return false;
    }
    
    // Get audio client
    hr = m_impl->pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                                   nullptr, reinterpret_cast<void**>(&m_impl->pAudioClient));
    if (FAILED(hr)) {
        std::cerr << "Failed to activate audio client" << std::endl;
        return false;
    }
    
    // Get the mix format
    hr = m_impl->pAudioClient->GetMixFormat(&m_impl->pWaveFormat);
    if (FAILED(hr)) {
        std::cerr << "Failed to get mix format" << std::endl;
        return false;
    }
    
    // Create event for audio capture
    m_impl->captureEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_impl->captureEvent == nullptr) {
        std::cerr << "Failed to create capture event" << std::endl;
        return false;
    }
    
    // Initialize the audio client for loopback capture
    hr = m_impl->pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        0, 0, m_impl->pWaveFormat, nullptr);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client for loopback" << std::endl;
        return false;
    }
    
    // Set the event handle
    hr = m_impl->pAudioClient->SetEventHandle(m_impl->captureEvent);
    if (FAILED(hr)) {
        std::cerr << "Failed to set event handle" << std::endl;
        return false;
    }
    
    // Get the buffer size
    hr = m_impl->pAudioClient->GetBufferSize(&m_impl->bufferFrameCount);
    if (FAILED(hr)) {
        std::cerr << "Failed to get buffer size" << std::endl;
        return false;
    }
    
    // Get the capture client
    hr = m_impl->pAudioClient->GetService(__uuidof(IAudioCaptureClient),
                                         reinterpret_cast<void**>(&m_impl->pCaptureClient));
    if (FAILED(hr)) {
        std::cerr << "Failed to get capture client" << std::endl;
        return false;
    }
    
    // Start the capture
    hr = m_impl->pAudioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Failed to start audio capture" << std::endl;
        return false;
    }
    
    m_impl->wasapiInitialized = true;
    m_audioAvailable = true;
    
    std::cout << "WASAPI loopback capture initialized successfully" << std::endl;
    std::cout << "Capturing at sample rate: " << m_impl->pWaveFormat->nSamplesPerSec << " Hz" << std::endl;
    std::cout << "Channels: " << m_impl->pWaveFormat->nChannels << std::endl;
    std::cout << "Bits per sample: " << m_impl->pWaveFormat->wBitsPerSample << std::endl;
    
#else
    // On non-Windows platforms, fall back to test data
    std::cout << "System audio capture not implemented for this platform." << std::endl;
    std::cout << "Using test audio data instead." << std::endl;
    m_audioAvailable = true;
#endif
    
    std::cout << "Audio processor initialized" << std::endl;
    return true;
}

void AudioProcessor::shutdown()
{
#ifdef _WIN32
    if (m_impl->wasapiInitialized) {
        // Stop audio capture
        if (m_impl->pAudioClient) {
            m_impl->pAudioClient->Stop();
        }
        
        // Release WASAPI resources
        if (m_impl->pCaptureClient) {
            m_impl->pCaptureClient->Release();
            m_impl->pCaptureClient = nullptr;
        }
        
        if (m_impl->pAudioClient) {
            m_impl->pAudioClient->Release();
            m_impl->pAudioClient = nullptr;
        }
        
        if (m_impl->pWaveFormat) {
            CoTaskMemFree(m_impl->pWaveFormat);
            m_impl->pWaveFormat = nullptr;
        }
        
        if (m_impl->pDevice) {
            m_impl->pDevice->Release();
            m_impl->pDevice = nullptr;
        }
        
        if (m_impl->pEnumerator) {
            m_impl->pEnumerator->Release();
            m_impl->pEnumerator = nullptr;
        }
        
        if (m_impl->captureEvent) {
            CloseHandle(m_impl->captureEvent);
            m_impl->captureEvent = nullptr;
        }
        
        CoUninitialize();
        m_impl->wasapiInitialized = false;
    }
#endif
    
    m_audioAvailable = false;
    std::cout << "Audio processor shutdown" << std::endl;
}

// Logarithmic scale function to improve dynamic range
float logScale(float value, float min_value = 0.0001f) {
    // Ensure value is positive and not too small
    value = std::max(value, min_value);
    
    // Apply logarithmic scaling (ln(x+1))
    float scaled = log(value + 1.0f) / log(2.0f);
    
    // Normalize to 0-1 range
    return scaled;
}

// Dynamic range compression function
float dynamicRangeCompression(float value, float threshold = 0.3f, float ratio = 0.5f) {
    if (value <= threshold) {
        return value;
    } else {
        return threshold + (value - threshold) * ratio;
    }
}

void AudioProcessor::update()
{
    if (!m_audioAvailable) {
        return;
    }
    
#ifdef _WIN32
    if (m_impl->wasapiInitialized) {
        // Wait for capture event (with a timeout)
        DWORD waitResult = WaitForSingleObject(m_impl->captureEvent, 10); // 10ms timeout
        
        // Capture audio data
        UINT32 packetLength = 0;
        HRESULT hr = m_impl->pCaptureClient->GetNextPacketSize(&packetLength);
        if (SUCCEEDED(hr) && packetLength > 0) {
            // Get the available data
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;
            
            hr = m_impl->pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (SUCCEEDED(hr)) {
                // Copy the data to our buffer (converting to float)
                UINT32 framesToProcess = std::min(numFramesAvailable, static_cast<UINT32>(m_frameSize));
                UINT32 bytesPerSample = m_impl->pWaveFormat->wBitsPerSample / 8;
                UINT32 numChannels = m_impl->pWaveFormat->nChannels;
                
                // Clear the waveform buffer
                std::fill(m_currentAudioData.waveform.begin(), m_currentAudioData.waveform.end(), 0.0f);
                
                // Convert the captured audio data to float
                for (UINT32 i = 0; i < framesToProcess; i++) {
                    float sampleSum = 0.0f;
                    
                    // Mix all channels to mono
                    for (UINT32 channel = 0; channel < numChannels; channel++) {
                        float sample = 0.0f;
                        
                        // Convert based on bit depth
                        if (bytesPerSample == 2) { // 16-bit
                            int16_t* samples = reinterpret_cast<int16_t*>(pData);
                            sample = static_cast<float>(samples[i * numChannels + channel]) / 32768.0f;
                        } else if (bytesPerSample == 4) { // 32-bit or float
                            if (m_impl->pWaveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
                                float* samples = reinterpret_cast<float*>(pData);
                                sample = samples[i * numChannels + channel];
                            } else {
                                int32_t* samples = reinterpret_cast<int32_t*>(pData);
                                sample = static_cast<float>(samples[i * numChannels + channel]) / 2147483648.0f;
                            }
                        }
                        
                        sampleSum += sample;
                    }
                    
                    // Average the channels
                    float monoSample = sampleSum / static_cast<float>(numChannels);
                    
                    // Store in waveform buffer (if within range)
                    if (i < m_currentAudioData.waveform.size()) {
                        m_currentAudioData.waveform[i] = monoSample;
                    }
                }
                
                // Release the buffer
                m_impl->pCaptureClient->ReleaseBuffer(numFramesAvailable);
                
                // Calculate RMS energy of the signal for better detection
                float rmsEnergy = 0.0f;
                for (size_t i = 0; i < m_currentAudioData.waveform.size(); i++) {
                    rmsEnergy += m_currentAudioData.waveform[i] * m_currentAudioData.waveform[i];
                }
                rmsEnergy = sqrt(rmsEnergy / m_currentAudioData.waveform.size());
                
                // Apply improved dynamic range processing
                
                // 1. Apply logarithmic scaling for better responsiveness at high volumes
                float logRmsEnergy = logScale(rmsEnergy);
                
                // 2. Apply dynamic range compression to control peaks
                float compressedRmsEnergy = dynamicRangeCompression(logRmsEnergy, 0.3f, 0.6f);
                
                // 3. Apply a moderate boost to increase sensitivity for quieter sounds
                const float sensitivityBoost = 1.0f; // Reduced from 2.0f to prevent maxing out
                float processedEnergy = compressedRmsEnergy * sensitivityBoost;
                
                // 4. Ensure the value stays in 0-1 range
                processedEnergy = std::min(1.0f, processedEnergy);
                
                // Debug output occasionally to see actual levels
                static int frameCount = 0;
                if (frameCount++ % 500 == 0) {
                    std::cout << "Raw audio RMS energy: " << rmsEnergy 
                              << " | Log-scaled: " << logRmsEnergy
                              << " | Compressed: " << compressedRmsEnergy
                              << " | Final: " << processedEnergy << std::endl;
                }
                
                // Perform FFT on the captured data to get the spectrum
                // In a real implementation, you would use a proper FFT library here
                // For now, we'll use a simplified approximation of frequency bands
                
                // Clear spectrum first
                std::fill(m_currentAudioData.spectrum.begin(), m_currentAudioData.spectrum.end(), 0.0f);
                
                // Very basic frequency analysis by checking different parts of the waveform
                // This is not a real FFT but gives us some approximation to work with
                
                // For a real application, you should implement a proper FFT here
                // This simple approach just divides the audio buffer into segments
                const int numBands = m_currentAudioData.spectrum.size();
                const int samplesPerBand = m_currentAudioData.waveform.size() / numBands;
                
                for (int band = 0; band < numBands; band++) {
                    float bandEnergy = 0.0f;
                    
                    // Calculate energy in this band's segment of the waveform
                    for (int i = 0; i < samplesPerBand; i++) {
                        int sampleIndex = band * samplesPerBand + i;
                        if (sampleIndex < m_currentAudioData.waveform.size()) {
                            float sample = m_currentAudioData.waveform[sampleIndex];
                            bandEnergy += sample * sample;
                        }
                    }
                    
                    // Normalize
                    bandEnergy = sqrt(bandEnergy / samplesPerBand);
                    
                    // Apply same processing as the overall energy
                    bandEnergy = logScale(bandEnergy);
                    bandEnergy = dynamicRangeCompression(bandEnergy, 0.3f, 0.6f);
                    bandEnergy *= sensitivityBoost;
                    
                    // Store in spectrum
                    m_currentAudioData.spectrum[band] = std::min(1.0f, bandEnergy);
                    
                    // Apply a curve to make the spectrum more visually interesting
                    // We'll boost lower frequencies to make bass more prominent
                    float freq = static_cast<float>(band) / numBands;
                    
                    if (freq < 0.1f) { // Bass frequencies (0-10%)
                        m_currentAudioData.spectrum[band] *= 1.2f; // Reduced from 1.5f
                    } else if (freq < 0.3f) { // Low-mid (10-30%)
                        m_currentAudioData.spectrum[band] *= 1.1f; // Reduced from 1.2f
                    } else if (freq < 0.7f) { // Mid (30-70%)
                        m_currentAudioData.spectrum[band] *= 1.0f; // Reduced from 1.1f
                    }
                    
                    // Cap at 1.0
                    m_currentAudioData.spectrum[band] = std::min(1.0f, m_currentAudioData.spectrum[band]);
                }
                
                // Calculate energy levels for bass, mid, and treble based on our segmented bands
                const int numBands10Pct = std::max(1, numBands / 10);
                const int numBands30Pct = std::max(1, numBands * 3 / 10);
                const int numBands60Pct = std::max(1, numBands * 6 / 10);
                
                float bassSum = 0.0f;
                float midSum = 0.0f;
                float trebleSum = 0.0f;
                
                // Bass: first 10% of bands
                for (int i = 0; i < numBands10Pct; i++) {
                    bassSum += m_currentAudioData.spectrum[i];
                }
                
                // Mid: next 20% of bands (10%-30%)
                for (int i = numBands10Pct; i < numBands30Pct; i++) {
                    midSum += m_currentAudioData.spectrum[i];
                }
                
                // Treble: next 30% of bands (30%-60%)
                for (int i = numBands30Pct; i < numBands60Pct; i++) {
                    trebleSum += m_currentAudioData.spectrum[i];
                }
                
                // Normalize by count (with the same processing as before)
                m_currentAudioData.bass = std::min(1.0f, bassSum / numBands10Pct * 1.0f);  // Reduced from 1.2f
                m_currentAudioData.mid = std::min(1.0f, midSum / (numBands30Pct - numBands10Pct) * 1.0f);
                m_currentAudioData.treble = std::min(1.0f, trebleSum / (numBands60Pct - numBands30Pct) * 1.0f);
                
                // Calculate overall energy - give more weight to bass for a better "feel"
                // Use the processed RMS energy instead of recalculating
                m_currentAudioData.energy = processedEnergy;
                
                // Add smoothing with previous frame for a more stable visualization
                if (!m_audioHistory.empty()) {
                    const float smoothFactor = 0.5f; // Increased from 0.3f for more stability
                    
                    const AudioData& prevData = m_audioHistory.back();
                    m_currentAudioData.bass = prevData.bass * smoothFactor + m_currentAudioData.bass * (1.0f - smoothFactor);
                    m_currentAudioData.mid = prevData.mid * smoothFactor + m_currentAudioData.mid * (1.0f - smoothFactor);
                    m_currentAudioData.treble = prevData.treble * smoothFactor + m_currentAudioData.treble * (1.0f - smoothFactor);
                    m_currentAudioData.energy = prevData.energy * smoothFactor + m_currentAudioData.energy * (1.0f - smoothFactor);
                    
                    // Detect transients
                    float prevEnergy = prevData.energy;
                    float energyDelta = std::max(0.0f, m_currentAudioData.energy - prevEnergy);
                    // Apply logarithmic scaling to transients as well for better detection
                    m_currentAudioData.transient = logScale(energyDelta * 5.0f); // Adjusted from 10.0f
                }
            }
        } else {
            // If no audio is playing, gradually reduce energy levels
            m_currentAudioData.bass *= 0.95f;
            m_currentAudioData.mid *= 0.95f;
            m_currentAudioData.treble *= 0.95f;
            m_currentAudioData.energy *= 0.95f;
            m_currentAudioData.transient *= 0.9f;
            
            // Also reduce spectrum values
            for (size_t i = 0; i < m_currentAudioData.spectrum.size(); i++) {
                m_currentAudioData.spectrum[i] *= 0.95f;
            }
            
            // Clear waveform when no audio
            if (m_currentAudioData.energy < 0.01f) {
                std::fill(m_currentAudioData.waveform.begin(), m_currentAudioData.waveform.end(), 0.0f);
            }
        }
    } else
#endif
    {
        // Fallback to generated test data on non-Windows or if WASAPI failed
        generateTestData();
    }
    
    // Update audio history
    m_audioHistory.push_back(m_currentAudioData);
    
    // Keep history at desired size
    while (m_audioHistory.size() > m_historySize) {
        m_audioHistory.pop_front();
    }
}

// Helper method to generate test audio data (placeholder)
void AudioProcessor::generateTestData()
{
    static float phase = 0.0f;
    static float bassPhase = 0.0f;
    static float midPhase = 0.0f;
    static float treblePhase = 0.0f;
    
    // Log that we're using test data
    static int counter = 0;
    if (counter++ % 500 == 0) {
        std::cout << "USING TEST AUDIO DATA - No real audio capture available" << std::endl;
    }
    
    // Generate more pronounced waveform with higher amplitude
    for (int i = 0; i < m_frameSize; ++i) {
        // Mix frequencies for bass, mid, and treble with higher amplitudes
        float bassFreq = 100.0f; // 100 Hz
        float midFreq = 1000.0f; // 1 kHz
        float trebleFreq = 5000.0f; // 5 kHz
        
        float t = static_cast<float>(i) / m_sampleRate;
        
        // Generate separate components with higher amplitudes
        float bassSignal = 0.8f * sin(2.0f * M_PI * bassFreq * t + bassPhase); // Increased from 0.4f
        float midSignal = 0.6f * sin(2.0f * M_PI * midFreq * t + midPhase);    // Increased from 0.3f
        float trebleSignal = 0.4f * sin(2.0f * M_PI * trebleFreq * t + treblePhase); // Increased from 0.2f
        
        // Add a sweeping component to make it more interesting
        float sweepFreq = 500.0f + 500.0f * sin(phase * 0.1f);
        float sweepSignal = 0.5f * sin(2.0f * M_PI * sweepFreq * t + phase * 0.5f);
        
        // Mix all components - will be automatically clamped in visualization
        m_currentAudioData.waveform[i] = bassSignal + midSignal + trebleSignal + sweepSignal;
    }
    
    // Update phases for next frame with higher speeds
    phase += 0.2f;        // Increased from 0.1f
    bassPhase += 0.05f;   // Increased from 0.02f
    midPhase += 0.1f;     // Increased from 0.05f
    treblePhase += 0.15f; // Increased from 0.08f
    
    // Generate fake spectrum data with higher peaks
    for (size_t i = 0; i < m_currentAudioData.spectrum.size(); ++i) {
        float freq = static_cast<float>(i) * m_sampleRate / m_frameSize;
        
        // Shape the spectrum to have peaks at different frequencies
        float spectrum = 0.0f;
        
        // Bass peak around 100 Hz
        if (freq < m_bassFrequencyLimit) {
            spectrum = 0.9f * exp(-pow((freq - 100.0f) / 50.0f, 2.0f)); // Increased from 0.8f
        }
        // Mid peak around 1000 Hz
        else if (freq < m_midFrequencyLimit) {
            spectrum = 0.8f * exp(-pow((freq - 1000.0f) / 400.0f, 2.0f)); // Increased from 0.6f
        }
        // Treble peak around 5000 Hz
        else if (freq < m_maxFrequency) {
            spectrum = 0.7f * exp(-pow((freq - 5000.0f) / 2000.0f, 2.0f)); // Increased from 0.4f
        }
        
        // Add some time variation - more pronounced
        spectrum *= 0.7f + 0.5f * sin(phase * 0.2f + freq * 0.001f); // Increased variation
        
        m_currentAudioData.spectrum[i] = spectrum;
    }
    
    // Calculate energy levels from spectrum
    float bassEnergy = 0.0f;
    float midEnergy = 0.0f;
    float trebleEnergy = 0.0f;
    int bassCount = 0;
    int midCount = 0;
    int trebleCount = 0;
    
    for (size_t i = 0; i < m_currentAudioData.spectrum.size(); ++i) {
        float freq = static_cast<float>(i) * m_sampleRate / m_frameSize;
        
        if (freq < m_bassFrequencyLimit) {
            bassEnergy += m_currentAudioData.spectrum[i];
            bassCount++;
        }
        else if (freq < m_midFrequencyLimit) {
            midEnergy += m_currentAudioData.spectrum[i];
            midCount++;
        }
        else if (freq < m_maxFrequency) {
            trebleEnergy += m_currentAudioData.spectrum[i];
            trebleCount++;
        }
    }
    
    // Normalize and update audio data with higher values
    m_currentAudioData.bass = (bassCount > 0) ? std::min(1.0f, bassEnergy / bassCount * 1.5f) : 0.0f;
    m_currentAudioData.mid = (midCount > 0) ? std::min(1.0f, midEnergy / midCount * 1.5f) : 0.0f;
    m_currentAudioData.treble = (trebleCount > 0) ? std::min(1.0f, trebleEnergy / trebleCount * 1.5f) : 0.0f;
    
    // Overall energy - boosted
    m_currentAudioData.energy = std::min(1.0f, (m_currentAudioData.bass + m_currentAudioData.mid + m_currentAudioData.treble) / 2.5f);
    
    // Detect transients (sudden changes in energy) - more pronounced
    if (!m_audioHistory.empty()) {
        float prevEnergy = m_audioHistory.back().energy;
        m_currentAudioData.transient = std::min(1.0f, std::max(0.0f, m_currentAudioData.energy - prevEnergy) * 8.0f); // Increased from 5.0f
    }
}

} // namespace av 