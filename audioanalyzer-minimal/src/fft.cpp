#include "fft.h"
#include <cmath>
#include <algorithm>

static void fft_rec(std::vector<complexd>& a) {
    size_t n = a.size();
    if (n <= 1) return;
    std::vector<complexd> even(n/2), odd(n/2);
    for (size_t i=0;i<n/2;i++) { even[i]=a[i*2]; odd[i]=a[i*2+1]; }
    fft_rec(even); fft_rec(odd);
    for (size_t k=0;k<n/2;k++) {
        complexd t = std::polar(1.0, -2.0*M_PI*k/n) * odd[k];
        a[k] = even[k] + t;
        a[k+n/2] = even[k] - t;
    }
}

void fft(std::vector<complexd>& a) {
    // pad to power of two
    size_t n = a.size();
    size_t p2 = 1; while (p2 < n) p2 <<= 1;
    if (p2 != n) a.resize(p2, 0.0);
    fft_rec(a);
}

void ifft(std::vector<complexd>& a) {
    for (auto &v : a) v = std::conj(v);
    fft(a);
    for (auto &v : a) v = std::conj(v) / (double)a.size();
}
