#include "fft.h"
static constexpr float PI = 3.1415926f;



void FFT::Init(Index n)
{
	if (omega_.size() < n) {
		omega_.resize(n);
	}

	if (reversed.size() < n) {
		reversed.resize(n);
	}

	Index log_2_N = log(n) / log(2);

	for (Index i = 0; i < n; ++i) {
		omega_[i] = Complex(
			std::cos(2 * PI / n * i),
			std::sin(2 * PI / n * i)
		);

		reversed[i] = ReverseBit(i, log_2_N);
	}
	N = n;
}

void FFT::Transform(Complex* a, Index n, Complex* omega)
{
	
	Index k = 0;
	while ((1 << k) < n) k++; 

	// for (Index i = 0; i < n; i++) {
	// 	// reverse bit
	// 	int t = ReverseBit(i, k);
	// 	if (i < t) std::swap(a[i], a[t]); // rearrange 
	// }

	for (Index l = 2; l <= n; l <<= 1) {
		Index m = l / 2;

		// merge pass
		for (Complex* p = a; p != a + n; p += l) {
			for (Index i = 0; i < m; ++i) {
				// butterfly op
				Complex t = omega[n / l * i] * p[m + i];
				p[m + i] = p[i] - t;
				p[i] += t;
			}
		}
	}
}

void FFT::fft(Complex* target)
{
	Transform(target, N, &omega_[0]);
}

FFT::Index FFT::ReverseBit(Index i, Index log_2_N)
{
	Index res = 0;
	for (Index j = 0; j < log_2_N; j++) {
		res = (res << 1) + (i & 1);
		i >>= 1;
	}
	return res;
}

void FFT::fft(Complex* input, Index stride, Index offset)
{
	if (buffer.size() < N) {
		buffer.resize(N);
	}

	for (Index i = 0; i < N; i++)
	{
		buffer[i] = input[reversed[i] * stride + offset];
	}
	Transform(&buffer[0], N, &omega_[0]);

	for (Index i = 0; i < N; i++) {
		input[i * stride + offset] = buffer[i];
	}
}

void FFT::fft(Complex* input, Complex* buf, Index stride, Index offset)
{
	for (Index i = 0; i < N; i++)
	{
		buf[i] = input[reversed[i] * stride + offset];
	}
	Transform(&buf[0], N, &omega_[0]);
	for (Index i = 0; i < N; i++) {
		input[i * stride + offset] = buf[i];
	}
}
