#include "bpm_key_core.h"
#include <cstring>
#include <cstdlib>

extern "C" {

// BPM detect
__declspec(dllexport) float soundtouch_detectBPMFile(const char* wavPath) {
    try {
        return detect_bpm_from_wav(std::string(wavPath));
    } catch(...) {
        return 0.0f;
    }
}

// Key detect
// Returns malloc'ed C string; caller should call free_string()
__declspec(dllexport) const char* analyze_key(const char* wavPath) {
    try {
        std::string k = detect_key_from_wav(std::string(wavPath));
        char* out = (char*)std::malloc(k.size()+1);
        if (!out) return nullptr;
        std::memcpy(out, k.c_str(), k.size()+1);
        return out;
    } catch(...) {
        const char* unk = "Unknown";
        char* out = (char*)std::malloc(strlen(unk)+1);
        std::strcpy(out, unk);
        return out;
    }
}

__declspec(dllexport) void free_string(const char* ptr) {
    if (ptr) std::free((void*)ptr);
}

} // extern "C"
