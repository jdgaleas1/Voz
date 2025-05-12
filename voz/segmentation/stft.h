#pragma once

// Devuelve un puntero doble a espectrograma [frames][bins]
double** applySTFT(const short* pcmData, int totalSamples, int sampleRate,
    int frameSizeMs, int frameStrideMs,
    int& outNumFrames, int& outBins);
