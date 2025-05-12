#pragma once

#include <vector>
#include <string>

// Prototipos antiguos (si quieres mantenerlos, ok)
// std::vector<std::vector<double>> extractMFCC(...)

double** extractMFCC(double** spectrogram, int numFrames, int numBins, int sampleRate,
    int numCoefficients, int& outNumFrames, int& outNumCoeffs);
