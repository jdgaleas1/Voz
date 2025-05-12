// vad.cpp - Eliminación de silencios (VAD) con punteros y OpenMP
#include "vad.h"
#include <cmath>
#include <vector>
#include <omp.h>
#include <mutex>

short* applyVAD(const short* pcmData, int totalSamples, int sampleRate, int numChannels, int& outSamples)
{
    if (pcmData == nullptr || totalSamples <= 0) {
        outSamples = 0;
        return nullptr;
    }

    const int frameSize = sampleRate / 100; // 10ms por frame
    const double energyThreshold = 500.0;

    std::vector<short> tempResult; // Se usa temporalmente por tamaño variable
    std::mutex resultMutex;

#pragma omp parallel
    {
        std::vector<short> localBuffer;

#pragma omp for nowait
        for (int i = 0; i < totalSamples; i += frameSize * numChannels)
        {
            double energy = 0.0;
            int frameSamples = std::min(frameSize * numChannels, totalSamples - i);

            for (int j = 0; j < frameSamples; ++j)
            {
                energy += std::abs(pcmData[i + j]);
            }

            energy /= frameSamples;

            if (energy > energyThreshold)
            {
                localBuffer.insert(localBuffer.end(), pcmData + i, pcmData + i + frameSamples);
            }
        }

        // Copiar resultados locales al vector global con mutex
        if (!localBuffer.empty())
        {
            std::lock_guard<std::mutex> lock(resultMutex);
            tempResult.insert(tempResult.end(), localBuffer.begin(), localBuffer.end());
        }
    }

    // Copiar resultados a memoria dinámica final
    outSamples = static_cast<int>(tempResult.size());
    if (outSamples == 0) return nullptr;

    short* result = new short[outSamples];
    std::copy(tempResult.begin(), tempResult.end(), result);

    return result; // El llamador debe liberar con delete[]
}
