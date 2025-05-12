#pragma once

// Devuelve puntero a PCM, el cual debe liberarse con delete[]
short* decodeMP3toPCM(const char* mp3File, int& sampleRate, int& numChannels, int& numSamples);

// Guarda el PCM a un archivo WAV
void savePCMtoWAV(const char* wavFile, const short* pcmData, int numSamples, int sampleRate, int numChannels);
