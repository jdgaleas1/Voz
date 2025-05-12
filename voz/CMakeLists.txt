cmake_minimum_required(VERSION 3.15)
project(voz)

# Agregar ejecutable
add_executable(voz "voz.cpp" "voz.h"   "preprocessing/vad.cpp" "preprocessing/vad.h" "preprocessing/denoise.h" "preprocessing/denoise.cpp" "preprocessing/normalize.cpp" "preprocessing/normalize.h" "features/mfcc.cpp" "features/mfcc.h" "load_audio/audio_io.cpp" "load_audio/audio_io.h" "segmentation/stft.h" "segmentation/stft.cpp" "load_audio/libs.cpp" )

# Estándar de C++
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET voz PROPERTY CXX_STANDARD 20)
endif()

# OpenMP (Procesamiento en paralelo)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(voz PUBLIC OpenMP::OpenMP_CXX)
endif()

# Copiar audio.mp3 al directorio de salida después del build
add_custom_command(TARGET voz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/voz/audio.mp3"
    $<TARGET_FILE_DIR:voz>
)
