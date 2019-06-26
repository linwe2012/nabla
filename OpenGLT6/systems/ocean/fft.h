#ifndef _NABLA_SYSTEM_OCEAN_FFT_H_
#define _NABLA_SYSTEM_OCEAN_FFT_H_
#include <complex>
#include <vector>
class FFT {
public:
	using Complex = std::complex<float>;
	using Index = unsigned;

	void Init(Index n);

	void Transform(Complex* a, Index n, Complex* omega);

	void fft(Complex* target);

	Index ReverseBit(Index i, Index log_2_N);

	void fft(Complex* target, Index stride, Index offset);

	void fft(Complex* target, Complex* buffer, Index stride, Index offset);

	Index n() { return N; }

private:
	std::vector<Complex> omega_;
	std::vector<Index> reversed;
	std::vector<Complex> buffer; // for strided fft
	Index N;
};
#endif // !_NABLA_SYSTEM_OCEAN_FFT_H_
