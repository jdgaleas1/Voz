// Reducción de ruido con punteros y OpenMP
#include "denoise.h"
#include <algorithm>
#include <omp.h>
#include <cstdlib>

short* applyDenoise(const short* pcmData, int totalSamples, int numChannels)
{
    if (pcmData == nullptr || totalSamples <= 0) return nullptr;

    const int windowSize = 5;
    short* result = new short[totalSamples];

#pragma omp parallel for
    for (int i = 0; i < totalSamples; ++i)
    {
        int sum = 0;
        int count = 0;

        for (int j = -windowSize; j <= windowSize; ++j)
        {
            int idx = i + j;
            if (idx >= 0 && idx < totalSamples)
            {
                sum += pcmData[idx];
                count++;
            }
        }

        result[i] = static_cast<short>(sum / count);
    }

    return result; // El llamador debe liberar con delete[]
}
