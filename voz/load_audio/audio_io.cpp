#include "audio_io.h"
#include <iostream>
#include "minimp3.h"
#include "minimp3_ex.h"
#include "dr_wav.h"

short* decodeMP3toPCM(const char* mp3File, int& sampleRate, int& numChannels, int& numSamples) {
    mp3dec_t mp3d;
    mp3dec_file_info_t info;

    mp3dec_init(&mp3d);
    int load_result = mp3dec_load(&mp3d, mp3File, &info, nullptr, nullptr);

    if (load_result) {
        std::cerr << "Error al cargar o decodificar el MP3: " << mp3File << std::endl;
        numSamples = 0;
        return nullptr;
    }

    sampleRate = info.hz;
    numChannels = info.channels;
    numSamples = info.samples;

    short* pcmData = new short[numSamples];
    std::copy(info.buffer, info.buffer + numSamples, pcmData);

    free(info.buffer); // Liberar buffer original
    return pcmData;    // El llamador debe liberar con delete[]
}

void savePCMtoWAV(const char* wavFile, const short* pcmData, int numSamples, int sampleRate, int numChannels) {
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = numChannels;
    format.sampleRate = sampleRate;
    format.bitsPerSample = 16;

    drwav wav;
    if (!drwav_init_file_write(&wav, wavFile, &format, nullptr)) {
        std::cerr << "Error al abrir WAV para escritura: " << wavFile << std::endl;
        return;
    }

    drwav_write_pcm_frames(&wav, numSamples / numChannels, pcmData);
    drwav_uninit(&wav);

    std::cout << "Archivo WAV escrito exitosamente: " << wavFile << std::endl;
}
