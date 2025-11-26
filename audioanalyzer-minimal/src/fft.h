#pragma once
#include <complex>
#include <vector>

using complexd = std::complex<double>;

void fft(std::vector<complexd>& a);
void ifft(std::vector<complexd>& a);
