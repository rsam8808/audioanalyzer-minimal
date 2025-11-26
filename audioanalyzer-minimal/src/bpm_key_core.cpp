#include "bpm_key_core.h"
#include "wav_reader.h"
#include "fft.h"
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <sstream>

// ------------------ BPM detector (simple energy + autocorrelation) ------------------
float detect_bpm_from_wav(const std::string& path) {
    WAVData w = read_wav_mono_16bit(path);
    if (!w.ok) return 0.0f;
    const int sr = w.sampleRate;
    const std::vector<float>& s = w.samples;
    // compute short-time energy envelope (hop = 1024)
    const int frameSize = 2048;
    const int hop = 512;
    std::vector<float> env;
    for (size_t i=0;i+frameSize<=s.size(); i+=hop) {
        double e=0;
        for (int j=0;j<frameSize;j++) { double v=s[i+j]; e += v*v; }
        env.push_back((float)std::sqrt(e));
    }
    if (env.size() < 4) return 0.0f;
    // normalize
    float maxv = *std::max_element(env.begin(), env.end());
    if (maxv <= 0.0f) return 0.0f;
    for (auto &v: env) v /= maxv;

    // autocorrelation on envelope
    int N = (int)env.size();
    int maxLag = std::min(N-1, (int)(sr * 60 / 60 / (float)hop)); // 60 BPM lower bound => long lag
    int minLag = std::max(1, (int)(sr * 60 / 200 / (float)hop)); // 200 BPM upper bound
    // to get in frames space:
    minLag = std::max(1, (int)( (60.0f/200.0f) * sr / hop ));
    maxLag = std::min(N-1, (int)( (60.0f/50.0f) * sr / hop )); // 50 BPM lower bound

    std::vector<double> ac(maxLag+1,0.0);
    for (int lag=minLag; lag<=maxLag; ++lag) {
        double sum=0;
        for (int i=0;i+lag<N;i++) sum += env[i]*env[i+lag];
        ac[lag] = sum;
    }
    // find lag with max ac
    int bestLag = minLag;
    double bestVal = ac[minLag];
    for (int lag=minLag+1; lag<=maxLag; ++lag) {
        if (ac[lag] > bestVal) { bestVal = ac[lag]; bestLag = lag; }
    }
    float bpm = 60.0f * (float)sr / ( (float)bestLag * (float)hop );
    // clamp reasonable
    if (bpm < 40) bpm *= 2.0f;
    if (bpm > 220) bpm /= 2.0f;
    if (bpm < 30 || bpm > 300) return 0.0f;
    return bpm;
}

// ------------------ Key detection (chroma + K-S template matching) ------------------
static const double majorProfile[12] = {6.35,2.23,3.48,2.33,4.38,4.09,2.52,5.19,2.39,3.66,2.29,2.88};
static const double minorProfile[12] = {6.33,2.68,3.52,5.38,2.6,3.53,2.54,4.75,3.98,2.69,3.34,3.17};

static int freq_to_bin(int k, int N, int sr) {
    // return center freq of bin k
    return (int)((double)k * sr / (double)N);
}

std::string detect_key_from_wav(const std::string& path) {
    WAVData w = read_wav_mono_16bit(path);
    if (!w.ok) return "Unknown";
    const int sr = w.sampleRate;
    const std::vector<float>& s = w.samples;
    // compute STFT frames and chroma
    int frameSize = 4096;
    int hop = 2048;
    int Nframes = (int)std::max(1, (int)((s.size()-frameSize)/hop + 1));
    std::vector<double> chroma(12, 0.0);
    std::vector<complexd> buf(frameSize);
    for (int f=0; f<Nframes; ++f) {
        int off = f*hop;
        for (int i=0;i<frameSize;i++) {
            double win = 0.5*(1 - cos(2*M_PI*i/(frameSize-1))); // hann
            double val = (off + i < (int)s.size()) ? s[off+i] * win : 0.0;
            buf[i] = complexd(val, 0.0);
        }
        fft(buf);
        int bins = (int)buf.size()/2;
        for (int k=0;k<bins;k++) {
            double mag = std::abs(buf[k]);
            double freq = (double)k * sr / (double)buf.size();
            if (freq < 55.0 || freq > 5000.0) continue;
            double c = 12.0 * log2(freq / 440.0) + 69.0; // midi note (approx)
            int midi = (int)std::round(c);
            int pc = (midi + 120) % 12;
            chroma[pc] += mag;
        }
    }
    // normalize chroma
    double sum = 0; for (double v: chroma) sum += v; if (sum<=0) return "Unknown";
    for (double &v: chroma) v /= sum;

    // correlate with rotated templates
    double bestScore = -1e9;
    int bestRoot = 0;
    bool bestIsMajor = true;
    for (int root=0; root<12; ++root) {
        // major
        double scoreM = 0;
        for (int i=0;i<12;i++) {
            int idx = (i + root) % 12;
            scoreM += chroma[idx] * majorProfile[i];
        }
        if (scoreM > bestScore) { bestScore = scoreM; bestRoot = root; bestIsMajor = true; }
        // minor
        double scorem = 0;
        for (int i=0;i<12;i++) {
            int idx = (i + root) % 12;
            scorem += chroma[idx] * minorProfile[i];
        }
        if (scorem > bestScore) { bestScore = scorem; bestRoot = root; bestIsMajor = false; }
    }
    // map root to name
    const char* namesSharp[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    std::ostringstream ss;
    ss << namesSharp[bestRoot];
    if (!bestIsMajor) ss << "m";
    return ss.str();
}
