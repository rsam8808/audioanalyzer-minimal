#include "wav_reader.h"
#include <fstream>
#include <cstdint>
#include <cstring>

static uint32_t read_u32(std::ifstream &f) {
    uint32_t v; f.read(reinterpret_cast<char*>(&v), 4); return v;
}
static uint16_t read_u16(std::ifstream &f) {
    uint16_t v; f.read(reinterpret_cast<char*>(&v), 2); return v;
}

WAVData read_wav_mono_16bit(const std::string& path) {
    WAVData out;
    std::ifstream f(path, std::ios::binary);
    if (!f) { out.error = "Cannot open file"; return out; }

    char riff[4]; f.read(riff,4);
    if (std::strncmp(riff,"RIFF",4)!=0) { out.error="Not WAV (no RIFF)"; return out; }
    read_u32(f); // file size
    char wave[4]; f.read(wave,4);
    if (std::strncmp(wave,"WAVE",4)!=0) { out.error="Not WAV (no WAVE)"; return out; }

    // parse chunks
    bool fmt_ok=false, data_ok=false;
    int16_t audioFormat=0;
    uint32_t dataSize=0;
    uint16_t blockAlign=0;
    uint16_t bitsPerSample=0;
    uint32_t byteRate=0;
    uint32_t sampleRate=0;
    uint16_t channels=0;

    while (f && (!fmt_ok || !data_ok)) {
        char id[4]; if(!f.read(id,4)) break;
        uint32_t sz = read_u32(f);
        if (std::strncmp(id,"fmt ",4)==0) {
            fmt_ok = true;
            audioFormat = read_u16(f);
            channels = read_u16(f);
            sampleRate = read_u32(f);
            byteRate = read_u32(f);
            blockAlign = read_u16(f);
            bitsPerSample = read_u16(f);
            // skip remainder
            if (sz > 16) f.seekg(sz - 16, std::ios::cur);
        } else if (std::strncmp(id,"data",4)==0) {
            data_ok = true;
            dataSize = sz;
            // read data then break
            if (!fmt_ok) { out.error="fmt before data missing"; return out; }
            if (audioFormat != 1) { out.error="Only PCM supported"; return out; }
            if (bitsPerSample != 16) { out.error="Only 16-bit WAV supported"; return out; }

            size_t sampleCount = dataSize / (bitsPerSample/8) / channels;
            out.samples.reserve(sampleCount);
            for (size_t i=0;i<sampleCount;i++) {
                int32_t acc = 0;
                for (uint16_t ch=0; ch<channels; ++ch) {
                    int16_t s=0; f.read(reinterpret_cast<char*>(&s), 2);
                    acc += s;
                }
                float mono = (float)acc / (float)(channels * 32768.0f);
                out.samples.push_back(mono);
            }
        } else {
            // skip chunk
            f.seekg(sz, std::ios::cur);
        }
    }

    if (!data_ok) { out.error="No data chunk"; return out; }
    out.sampleRate = (int)sampleRate;
    out.channels = (int)channels;
    out.ok = true;
    return out;
}
