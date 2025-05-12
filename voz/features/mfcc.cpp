// Extracción de MFCC con punteros y OpenMP
#include "mfcc.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double hzToMel(double hz) {
    return 2595.0 * std::log10(1.0 + hz / 700.0);
}

static double melToHz(double mel) {
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
}

static double** createMelFilterbank(int sampleRate, int numFilters, int fftSize)
{
    int numBins = fftSize / 2 + 1;
    double** filterbank = new double* [numFilters];
    for (int i = 0; i < numFilters; ++i)
        filterbank[i] = new double[numBins]();

    double lowMel = hzToMel(300.0);
    double highMel = hzToMel(3400.0);

    std::vector<double> melPoints(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i)
        melPoints[i] = lowMel + (highMel - lowMel) * i / (numFilters + 1);

    std::vector<int> bin(melPoints.size());
    for (size_t i = 0; i < melPoints.size(); ++i)
        bin[i] = static_cast<int>(std::floor((melToHz(melPoints[i]) * (fftSize + 1)) / sampleRate));

    for (int i = 0; i < numFilters; ++i) {
        for (int j = bin[i]; j < bin[i + 1]; ++j)
            filterbank[i][j] = (j - bin[i]) / static_cast<double>(bin[i + 1] - bin[i]);
        for (int j = bin[i + 1]; j < bin[i + 2]; ++j)
            filterbank[i][j] = (bin[i + 2] - j) / static_cast<double>(bin[i + 2] - bin[i + 1]);
    }

    return filterbank;
}

double** extractMFCC(double** spectrogram, int numFrames, int numBins, int sampleRate, int numCoefficients, int& outNumFrames, int& outNumCoeffs)
{
    const int numFilters = 26;
    double** mfcc = new double* [numFrames];
    for (int i = 0; i < numFrames; ++i)
        mfcc[i] = new double[numCoefficients];

    double** filterbank = createMelFilterbank(sampleRate, numFilters, numBins * 2);

#pragma omp parallel for
    for (int f = 0; f < numFrames; ++f)
    {
        double* filterEnergies = new double[numFilters]();

        // Calcular energías Mel
        for (int i = 0; i < numFilters; ++i) {
            for (int j = 0; j < numBins; ++j)
                filterEnergies[i] += spectrogram[f][j] * filterbank[i][j];
            if (filterEnergies[i] < 1.0) filterEnergies[i] = 1.0;
            filterEnergies[i] = std::log(filterEnergies[i]);
        }

        // DCT tipo II
        for (int k = 0; k < numCoefficients; ++k) {
            mfcc[f][k] = 0.0;
            for (int n = 0; n < numFilters; ++n) {
                mfcc[f][k] += filterEnergies[n] * std::cos(M_PI * k * (n + 0.5) / numFilters);
            }
        }

        delete[] filterEnergies;
    }

    // Limpiar filtro Mel
    for (int i = 0; i < numFilters; ++i) delete[] filterbank[i];
    delete[] filterbank;

    outNumFrames = numFrames;
    outNumCoeffs = numCoefficients;
    return mfcc; // El llamador debe liberar con delete[]
}
