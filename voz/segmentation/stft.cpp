#include "stft.h"
#include <cmath>
#include <omp.h>
#include <cstdlib>
#include <complex>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Función para realizar la FFT de Cooley-Tukey
void fft(std::vector<std::complex<double>>& data) {
    int n = data.size();
    if (n <= 1) return;

    // Dividir el conjunto de datos en partes pares e impares
    std::vector<std::complex<double>> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = data[2 * i];
        odd[i] = data[2 * i + 1];
    }

    // Llamada recursiva para cada mitad
    fft(even);
    fft(odd);

    // Combinar los resultados
    for (int i = 0; i < n / 2; ++i) {
        std::complex<double> t = std::polar(1.0, -2 * M_PI * i / n) * odd[i];
        data[i] = even[i] + t;
        data[i + n / 2] = even[i] - t;
    }
}

double** applySTFT(const short* pcmData, int totalSamples, int sampleRate, int frameSizeMs, int frameStrideMs, int& outNumFrames, int& outBins)
{
    if (pcmData == nullptr || totalSamples <= 0) {
        outNumFrames = 0;
        outBins = 0;
        return nullptr;
    }

    const int frameSize = sampleRate * frameSizeMs / 1000;
    const int frameStride = sampleRate * frameStrideMs / 1000;
    const int numFrames = (totalSamples - frameSize) / frameStride + 1;
    const int numBins = frameSize / 2;

    double** spectrogram = new double* [numFrames];
    for (int i = 0; i < numFrames; ++i)
        spectrogram[i] = new double[numBins];

    // Paralelización con OpenMP
#pragma omp parallel
    {
        // Usar una estructura compleja para la FFT
        std::vector<std::complex<double>> fftResult(frameSize);

#pragma omp for
        for (int i = 0; i < numFrames; ++i)
        {
            int start = i * frameStride;

            // Aplicar ventana Hamming y preparar la entrada para FFT
            for (int j = 0; j < frameSize; ++j)
            {
                double windowed = pcmData[start + j] * (0.54 - 0.46 * std::cos(2 * M_PI * j / (frameSize - 1)));
                fftResult[j] = std::complex<double>(windowed, 0.0);
            }

            // Realizar la FFT (sin FFTW3)
            fft(fftResult);

            // Magnitud del espectro
            for (int j = 0; j < numBins; ++j)
            {
                double real = fftResult[j].real();
                double imag = fftResult[j].imag();
                spectrogram[i][j] = std::sqrt(real * real + imag * imag);
            }
        }
    }

    outNumFrames = numFrames;
    outBins = numBins;
    return spectrogram; // El llamador debe liberar con delete[]
}
