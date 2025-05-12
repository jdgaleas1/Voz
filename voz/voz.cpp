#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "load_audio/audio_io.h"
#include "preprocessing/vad.h"
#include "preprocessing/denoise.h"
#include "preprocessing/normalize.h"
#include "segmentation/stft.h" 
#include "features/mfcc.h"

int main() {
    using namespace std::chrono;

    auto start = steady_clock::now();

    const char* inputMP3 = "audio.mp3";
    const char* outputWAV = "audio.wav";

    int sampleRate = 0, numChannels = 0, numSamples = 0;
    short* pcmData = nullptr;

    // Decodificar MP3 en hilo
    std::thread decodeThread([&]() {
        pcmData = decodeMP3toPCM(inputMP3, sampleRate, numChannels, numSamples);
        });
    decodeThread.join();

    if (!pcmData) {
        std::cerr << "Fallo al decodificar MP3.\n";
        return 1;
    }

    // Guardar WAV en hilo
    std::thread saveThread(savePCMtoWAV, outputWAV, pcmData, numSamples, sampleRate, numChannels);
    saveThread.join();

    // Preprocesamiento
    int vadSamples = 0;
    short* vadData = applyVAD(pcmData, numSamples, sampleRate, numChannels, vadSamples);
    delete[] pcmData;
    if (!vadData) {
        std::cerr << "Fallo en VAD.\n";
        return 1;
    }

    short* denoisedData = applyDenoise(vadData, vadSamples, numChannels);
    delete[] vadData;
    if (!denoisedData) {
        std::cerr << "Fallo en denoise.\n";
        return 1;
    }

    short* normalizedData = applyNormalization(denoisedData, vadSamples);
    delete[] denoisedData;
    if (!normalizedData) {
        std::cerr << "Fallo en normalización.\n";
        return 1;
    }

    // Segmentación (STFT)
    int numFrames = 0, numBins = 0;
    double** spectrogram = applySTFT(normalizedData, vadSamples, sampleRate, 25, 10, numFrames, numBins);
    delete[] normalizedData;
    if (!spectrogram) {
        std::cerr << "Fallo en STFT.\n";
        return 1;
    }

    std::cout << "Espectrograma: " << numFrames << " frames x " << numBins << " bins.\n";

    // Extracción de características
    int mfccFrames = 0, mfccCoeffs = 0;
    double** mfccs = extractMFCC(spectrogram, numFrames, numBins, sampleRate, 13, mfccFrames, mfccCoeffs);

    if (!mfccs) {
        std::cerr << "Fallo en extracción de MFCC.\n";
        return 1;
    }

    std::cout << "MFCCs extraídos: " << mfccFrames << " frames x " << mfccCoeffs << " coeficientes.\n";

    // Guardar CSV
    std::ofstream csv("output_mfcc.csv");
    for (int i = 0; i < mfccFrames; ++i) {
        for (int j = 0; j < mfccCoeffs; ++j) {
            csv << mfccs[i][j];
            if (j < mfccCoeffs - 1) csv << ",";
        }
        csv << "\n";
    }
    csv.close();

    // Guardar BIN
    std::ofstream bin("output_mfcc.bin", std::ios::binary);
    bin.write(reinterpret_cast<const char*>(&mfccFrames), sizeof(int));
    bin.write(reinterpret_cast<const char*>(&mfccCoeffs), sizeof(int));
    for (int i = 0; i < mfccFrames; ++i)
        bin.write(reinterpret_cast<const char*>(mfccs[i]), mfccCoeffs * sizeof(double));
    bin.close();

    // Liberar memoria
    for (int i = 0; i < numFrames; ++i) delete[] spectrogram[i];
    delete[] spectrogram;

    for (int i = 0; i < mfccFrames; ++i) delete[] mfccs[i];
    delete[] mfccs;



    std::cout << "Proceso completado exitosamente.\n";

    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Tiempo total de ejecución: " << duration.count() << " ms\n";

    return 0;
}

