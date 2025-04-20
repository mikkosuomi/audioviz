#include "AudioProcessor.h"
#include <cmath>
#include <vector>
#include <complex>
#include <algorithm>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace av {

// Simple FFT implementation (placeholder for a real FFT library)
// In a production system, you would use a real FFT library like FFTW or kiss_fft
class FFTAnalyzer {
public:
    // Perform FFT on the input data and return the magnitude spectrum
    static std::vector<float> computeFFT(const std::vector<float>& input) {
        // Ensure input size is a power of 2
        int size = nextPowerOf2(input.size());
        
        // Pad input if necessary
        std::vector<std::complex<float>> data(size);
        for (size_t i = 0; i < input.size(); ++i) {
            data[i] = std::complex<float>(input[i], 0.0f);
        }
        
        // Apply window function (Hanning)
        applyWindow(data, input.size());
        
        // Perform FFT
        fft(data);
        
        // Compute magnitude spectrum
        std::vector<float> spectrum(size / 2 + 1);
        spectrum[0] = std::abs(data[0].real()) / size;  // DC component
        
        for (int i = 1; i < size / 2; ++i) {
            spectrum[i] = 2.0f * std::sqrt(data[i].real() * data[i].real() + 
                                          data[i].imag() * data[i].imag()) / size;
        }
        
        spectrum[size / 2] = std::abs(data[size / 2].real()) / size;  // Nyquist component
        
        return spectrum;
    }
    
private:
    // Find the next power of 2
    static int nextPowerOf2(int n) {
        int power = 1;
        while (power < n) {
            power *= 2;
        }
        return power;
    }
    
    // Apply a Hanning window function
    static void applyWindow(std::vector<std::complex<float>>& data, int inputSize) {
        for (int i = 0; i < inputSize; ++i) {
            float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (inputSize - 1)));
            data[i] *= window;
        }
    }
    
    // Simple recursive FFT implementation (Cooley-Tukey algorithm)
    static void fft(std::vector<std::complex<float>>& data) {
        int n = data.size();
        
        if (n <= 1) {
            return;
        }
        
        // Divide
        std::vector<std::complex<float>> even(n / 2);
        std::vector<std::complex<float>> odd(n / 2);
        
        for (int i = 0; i < n / 2; ++i) {
            even[i] = data[i * 2];
            odd[i] = data[i * 2 + 1];
        }
        
        // Conquer
        fft(even);
        fft(odd);
        
        // Combine
        for (int k = 0; k < n / 2; ++k) {
            float angle = -2.0f * M_PI * k / n;
            std::complex<float> t(std::cos(angle), std::sin(angle));
            t = t * odd[k];
            data[k] = even[k] + t;
            data[k + n / 2] = even[k] - t;
        }
    }
};

} // namespace av 