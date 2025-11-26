cmake_minimum_required(VERSION 3.15)
project(AudioAnalyzerMinimal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(audioanalyzer SHARED
    src/wav_reader.cpp
    src/fft.cpp
    src/bpm_key_core.cpp
    src/exports.cpp
)

target_include_directories(audioanalyzer PRIVATE src)

# Release flags
if(MSVC)
    target_compile_options(audioanalyzer PRIVATE /O2 /W3)
else()
    target_compile_options(audioanalyzer PRIVATE -O3 -Wall -Wextra)
endif()

# Optional: copy produced DLL to friendly names
add_custom_command(TARGET audioanalyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:audioanalyzer>
        ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/soundtouch.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:audioanalyzer>
        ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/libkeyfinder.dll
)
