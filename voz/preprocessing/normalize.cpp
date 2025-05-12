// Normalización de volumen con punteros y OpenMP
#include "normalize.h"
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <cstdlib> // Para std::abs

short* applyNormalization(const short* pcmData, int totalSamples)
{
    if (pcmData == nullptr || totalSamples <= 0) return nullptr;

    short* result = new short[totalSamples];

    short maxVal = 1;

    // Paso 1: Encontrar valor absoluto máximo en paralelo
#pragma omp parallel
    {
        short localMax = 1;

#pragma omp for nowait
        for (int i = 0; i < totalSamples; ++i)
        {
            localMax = std::max(localMax, static_cast<short>(std::abs(pcmData[i])));
        }

#pragma omp critical
        {
            maxVal = std::max(maxVal, localMax);
        }
    }

    // Paso 2: Calcular ganancia
    double targetAmplitude = 0.9 * 32767;
    double gain = targetAmplitude / maxVal;

    // Paso 3: Aplicar normalización en paralelo
#pragma omp parallel for
    for (int i = 0; i < totalSamples; ++i)
    {
        int normalized = static_cast<int>(pcmData[i] * gain);
        normalized = std::clamp(normalized, -32767, 32767);
        result[i] = static_cast<short>(normalized);
    }

    return result; // El llamador debe liberar con delete[]
}
