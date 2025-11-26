#pragma once
#include <string>
#include <vector>

struct WAVData {
    int sampleRate = 0;
    int channels = 0;
    std::vector<float> samples; // normalized -1.0 .. 1.0 mono (averaged if stereo)
    bool ok = false;
    std::string error;
};

WAVData read_wav_mono_16bit(const std::string& path);
