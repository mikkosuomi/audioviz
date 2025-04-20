#include <SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstring>

// Audio callback and buffer size
const int AUDIO_BUFFER_SIZE = 1024;
const int SAMPLE_RATE = 48000;
const int FFT_SIZE = 1024;

// Waveform storage
std::vector<float> g_waveform(AUDIO_BUFFER_SIZE, 0.0f);
std::vector<float> g_spectrum(FFT_SIZE/2, 0.0f);
float g_energy = 0.0f;
float g_bass = 0.0f;
float g_mid = 0.0f;
float g_treble = 0.0f;

// Audio capture callback
void audioCallback(void* userdata, Uint8* stream, int len) {
    // Copy the audio data to our buffer
    if (len / sizeof(float) > AUDIO_BUFFER_SIZE) {
        len = AUDIO_BUFFER_SIZE * sizeof(float);
    }
    
    // Simple frequency analysis - using basic band calculations
    float* samples = (float*)stream;
    int sampleCount = len / sizeof(float);
    
    // Copy waveform data
    std::copy(samples, samples + std::min(sampleCount, AUDIO_BUFFER_SIZE), g_waveform.begin());
    
    // Calculate energy
    float totalEnergy = 0.0f;
    for (int i = 0; i < sampleCount; i++) {
        totalEnergy += samples[i] * samples[i];
    }
    // Normalize and apply smoothing
    g_energy = 0.9f * g_energy + 0.1f * std::min(1.0f, std::sqrt(totalEnergy / sampleCount) * 5.0f);
    
    // Simple frequency band analysis
    // This is a very simplified approach without FFT
    float bassSum = 0.0f;
    float midSum = 0.0f;
    float trebleSum = 0.0f;
    
    // For demonstration: analyze different parts of the buffer for different frequency bands
    // In a real application, you would use FFT for proper frequency analysis
    const int bassRange = sampleCount / 3;
    const int midRange = bassRange * 2;
    
    for (int i = 0; i < std::min(bassRange, sampleCount); i++) {
        bassSum += std::abs(samples[i]);
    }
    
    for (int i = bassRange; i < std::min(midRange, sampleCount); i++) {
        midSum += std::abs(samples[i]);
    }
    
    for (int i = midRange; i < sampleCount; i++) {
        trebleSum += std::abs(samples[i]);
    }
    
    // Normalize and smooth
    if (bassRange > 0) {
        float newBass = bassSum / bassRange * 3.0f;
        g_bass = 0.8f * g_bass + 0.2f * std::min(1.0f, newBass);
    }
    
    if (midRange - bassRange > 0) {
        float newMid = midSum / (midRange - bassRange) * 3.0f;
        g_mid = 0.8f * g_mid + 0.2f * std::min(1.0f, newMid);
    }
    
    if (sampleCount - midRange > 0) {
        float newTreble = trebleSum / (sampleCount - midRange) * 3.0f;
        g_treble = 0.8f * g_treble + 0.2f * std::min(1.0f, newTreble);
    }
    
    // Generate simple spectrum data (simulated)
    for (int i = 0; i < FFT_SIZE/2; i++) {
        float normalized = static_cast<float>(i) / (FFT_SIZE/2);
        // Create spectrum based on frequency bands
        float value = 0.0f;
        
        if (normalized < 0.33f) {
            // Bass range
            value = g_bass * (1.0f - normalized/0.33f);
        } else if (normalized < 0.66f) {
            // Mid range
            value = g_mid * (1.0f - (normalized-0.33f)/0.33f);
        } else {
            // Treble range
            value = g_treble * (1.0f - (normalized-0.66f)/0.34f);
        }
        
        // Add some variation
        value *= 0.5f + 0.5f * std::sin(normalized * 10.0f);
        
        // Smooth spectrum
        g_spectrum[i] = 0.7f * g_spectrum[i] + 0.3f * value;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Starting SDL Audio Reactive Visualization Program ===" << std::endl;
    
    // Initialize SDL with audio support
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "SDL Audio Reactive Visualization",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Set up audio specification
    SDL_AudioSpec desiredSpec;
    SDL_AudioSpec obtainedSpec;
    
    SDL_zero(desiredSpec);
    desiredSpec.freq = SAMPLE_RATE;
    desiredSpec.format = AUDIO_F32;
    desiredSpec.channels = 1;
    desiredSpec.samples = AUDIO_BUFFER_SIZE;
    desiredSpec.callback = audioCallback;
    
    // Open audio device
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 1, &desiredSpec, &obtainedSpec, 0);
    
    if (!audioDevice) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        std::cout << "Continuing with demo mode (no audio capture)" << std::endl;
    } else {
        std::cout << "Audio capture device opened successfully." << std::endl;
        std::cout << "Sample rate: " << obtainedSpec.freq << ", Buffer size: " << obtainedSpec.samples << std::endl;
        
        // Start audio capture
        SDL_PauseAudioDevice(audioDevice, 0);
    }
    
    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
    // Animation time
    float time = 0.0f;
    
    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // User presses a key
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_f:
                        // Toggle fullscreen
                        Uint32 flags = SDL_GetWindowFlags(window);
                        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                            SDL_SetWindowFullscreen(window, 0);
                        } else {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        break;
                }
            }
        }
        
        // If no audio is available, generate a waveform
        if (!audioDevice) {
            // Update waveform with simulated audio
            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
                float normalized = static_cast<float>(i) / AUDIO_BUFFER_SIZE;
                
                // Create a basic sine wave with some harmonics
                g_waveform[i] = 0.5f * sin(2.0f * M_PI * normalized * 2.0f + time) +
                               0.25f * sin(2.0f * M_PI * normalized * 4.0f + time * 1.5f) +
                               0.125f * sin(2.0f * M_PI * normalized * 8.0f + time * 2.0f);
            }
            
            // Simulate energy, bass, mid, treble
            g_energy = 0.5f + 0.5f * sin(time * 0.5f);
            g_bass = 0.6f + 0.4f * sin(time * 0.7f);
            g_mid = 0.5f + 0.5f * sin(time * 1.3f);
            g_treble = 0.4f + 0.6f * sin(time * 2.1f);
            
            // Update simulated spectrum
            for (int i = 0; i < FFT_SIZE/2; i++) {
                float normalized = static_cast<float>(i) / (FFT_SIZE/2);
                // Create spectrum based on frequency bands
                float value = 0.0f;
                
                if (normalized < 0.33f) {
                    // Bass range
                    value = g_bass * (1.0f - normalized/0.33f);
                } else if (normalized < 0.66f) {
                    // Mid range
                    value = g_mid * (1.0f - (normalized-0.33f)/0.33f);
                } else {
                    // Treble range
                    value = g_treble * (1.0f - (normalized-0.66f)/0.34f);
                }
                
                // Add some variation
                value *= 0.5f + 0.5f * sin(normalized * 10.0f + time);
                
                // Smooth spectrum
                g_spectrum[i] = 0.7f * g_spectrum[i] + 0.3f * value;
            }
        }
        
        // Update animation time
        time += 0.05f;
        
        // Get window dimensions
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        
        // Clear the screen with a background color based on audio energy
        // Use orange hue but vary brightness with energy
        Uint8 bgR = 255;
        Uint8 bgG = static_cast<Uint8>(128 + 127 * g_energy);
        Uint8 bgB = static_cast<Uint8>(64 * g_energy);
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 255);
        SDL_RenderClear(renderer);
        
        // == Draw waveform visualization ==
        // Draw background rectangle for waveform
        SDL_Rect waveRect = {20, 20, width - 40, height / 3};
        SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255); // Dark blue background
        SDL_RenderFillRect(renderer, &waveRect);
        
        // Draw border around waveform area - pulse with bass
        Uint8 borderThickness = 1 + static_cast<Uint8>(3 * g_bass);
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow border
        for (int i = 0; i < borderThickness; i++) {
            SDL_Rect borderRect = {waveRect.x - i, waveRect.y - i, 
                                 waveRect.w + i*2, waveRect.h + i*2};
            SDL_RenderDrawRect(renderer, &borderRect);
        }
        
        // Midpoint line
        int midY = waveRect.y + waveRect.h / 2;
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray for middle line
        SDL_RenderDrawLine(renderer, waveRect.x, midY, waveRect.x + waveRect.w, midY);
        
        // Draw waveform with thick lines
        Uint8 r = 255;
        Uint8 g = static_cast<Uint8>(g_energy * 255);
        Uint8 b = 0;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255); // Color based on energy
        
        // Calculate point coordinates
        std::vector<SDL_Point> points(AUDIO_BUFFER_SIZE);
        float xStep = static_cast<float>(waveRect.w) / (AUDIO_BUFFER_SIZE - 1);
        float yScale = waveRect.h / 2.0f * 0.9f; // Scale factor for amplitude (90% of half height)
        
        for (int i = 0; i < AUDIO_BUFFER_SIZE; ++i) {
            points[i].x = waveRect.x + static_cast<int>(i * xStep);
            points[i].y = midY - static_cast<int>(g_waveform[i] * yScale);
        }
        
        // Draw lines connecting points
        for (int i = 0; i < AUDIO_BUFFER_SIZE - 1; ++i) {
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
            
            // Draw thicker lines by drawing multiple offset lines
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y + 1, points[i + 1].x, points[i + 1].y + 1);
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y - 1, points[i + 1].x, points[i + 1].y - 1);
        }
        
        // == Draw spectrum visualization ==
        SDL_Rect spectrumRect = {20, waveRect.y + waveRect.h + 20, width - 40, height / 4};
        
        // Draw spectrum background
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderFillRect(renderer, &spectrumRect);
        
        // Draw spectrum border - pulse with treble
        borderThickness = 1 + static_cast<Uint8>(3 * g_treble);
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan border
        for (int i = 0; i < borderThickness; i++) {
            SDL_Rect borderRect = {spectrumRect.x - i, spectrumRect.y - i, 
                                 spectrumRect.w + i*2, spectrumRect.h + i*2};
            SDL_RenderDrawRect(renderer, &borderRect);
        }
        
        // Draw spectrum bars
        int barCount = std::min(128, (int)g_spectrum.size());
        float barWidth = static_cast<float>(spectrumRect.w) / barCount;
        
        for (int i = 0; i < barCount; i++) {
            float barHeight = g_spectrum[i] * spectrumRect.h;
            
            // Choose color based on frequency range (red for bass, green for mid, blue for high)
            Uint8 r, g, b;
            float normalized = static_cast<float>(i) / barCount;
            if (normalized < 0.33f) {
                r = 255;
                g = static_cast<Uint8>(normalized * 3 * 255);
                b = 0;
            } else if (normalized < 0.66f) {
                r = static_cast<Uint8>((1.0f - (normalized - 0.33f) * 3) * 255);
                g = 255;
                b = static_cast<Uint8>((normalized - 0.33f) * 3 * 255);
            } else {
                r = 0;
                g = static_cast<Uint8>((1.0f - (normalized - 0.66f) * 3) * 255);
                b = 255;
            }
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            
            SDL_Rect barRect = {
                spectrumRect.x + static_cast<int>(i * barWidth),
                spectrumRect.y + spectrumRect.h - static_cast<int>(barHeight),
                static_cast<int>(barWidth) - 1,
                static_cast<int>(barHeight)
            };
            
            SDL_RenderFillRect(renderer, &barRect);
        }
        
        // == Draw frequency band indicators ==
        int circleY = height - height / 6;
        int radius;
        
        // Bass circle - Red - at 1/4 width
        radius = 20 + static_cast<int>(g_bass * 60.0f);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius) {
                    SDL_RenderDrawPoint(renderer, width/4 + x, circleY + y);
                }
            }
        }
        
        // Mid circle - Green - at center
        radius = 20 + static_cast<int>(g_mid * 50.0f);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius) {
                    SDL_RenderDrawPoint(renderer, width/2 + x, circleY + y);
                }
            }
        }
        
        // Treble circle - Blue - at 3/4 width
        radius = 20 + static_cast<int>(g_treble * 40.0f);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius) {
                    SDL_RenderDrawPoint(renderer, width*3/4 + x, circleY + y);
                }
            }
        }
        
        // Draw energy bar at the bottom
        int barHeight = 20;
        int energyBarWidth = static_cast<int>(width * g_energy);
        
        // Energy bar background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect bgRect = {0, height - barHeight, width, barHeight};
        SDL_RenderFillRect(renderer, &bgRect);
        
        // Energy bar fill
        SDL_SetRenderDrawColor(renderer, 
                             static_cast<Uint8>(255 * (1.0f - g_energy)), 
                             static_cast<Uint8>(255 * g_energy), 
                             0, 255);
        SDL_Rect fillRect = {0, height - barHeight, energyBarWidth, barHeight};
        SDL_RenderFillRect(renderer, &fillRect);
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Output a message periodically
        static int frameCount = 0;
        if (++frameCount % 60 == 0) {
            std::cout << "Frame " << frameCount 
                     << " - Energy: " << g_energy 
                     << ", Bass: " << g_bass 
                     << ", Mid: " << g_mid 
                     << ", Treble: " << g_treble << std::endl;
        }
        
        // Add small delay
        SDL_Delay(16);
    }
    
    // Clean up audio if it was initialized
    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
    }
    
    // Destroy window and renderer
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    // Quit SDL subsystems
    SDL_Quit();
    
    std::cout << "=== SDL Audio Reactive Visualization Program Exited Normally ===" << std::endl;
    return 0;
} 