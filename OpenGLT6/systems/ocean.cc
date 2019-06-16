#include "ocean.h"
#include <complex>

namespace nabla {
using Complex = std::complex<float>;

struct ShaderData {
	glm::vec3 vertex;
	glm::vec3 normal;
	glm::vec2 htilde0;
};

struct PreComputeData {
	glm::vec3 orgin;
	glm::vec2 htilde0_conj;
};

struct OceanParameters::Data {
	Vector<ShaderData> shader_data;
	Vector<PreComputeData> precompute;
	Vector<unsigned> indices;
};

struct OceanSystem::Data {
	Vector<Complex> h_tilde;
	Vector<Complex> h_tilde_slopex;
	Vector<Complex> h_tilde_slopez;
	Vector<Complex> h_tilde_dx;
	Vector<Complex> h_tilde_dz;

	OceanParameters* active_ocean;

	void MakeSpace(OceanParameters& op);

	void EvaluateWavesFFT(OceanParameters& op, float t);
};

float OceanParameters::Kx(int n_prime) const
{
	return PI * (2 * n_prime - N) / length;
}

float OceanParameters::Kz(int m_prime) const
{
	return PI * (2 * m_prime - N) / length;
}

static float PreComputeOmega(const OceanParameters& op, int n_prime, int m_prime) {
	float w_0 = 2.0f * PI / 200.0f; //< Why????
	float kx = op.Kx(n_prime);
	float kz = op.Kz(m_prime);
	return floor(sqrt(op.g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

static float Phillips(const OceanParameters& op, int n_prime, int m_prime) {
	glm::vec2 k(
		op.Kx(n_prime),
		op.Kz(m_prime)
	);
	float k_length = k.length();
	if (k_length < 0.000001) return 0.0; // spare some computation
	float k_length2 = k_length * k_length;
	float k_length4 = k_length2 * k_length2;
	auto w = op.w;
	auto g = op.g;

	float k_dot_w = glm::normalize(glm::dot(k, w));
	float k_dot_w2 = k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w * k_dot_w;

	float w_length = w.length(); // the speed of the wind
	float L = w_length * w_length / g;
	float L2 = L * L;

	float damping = 0.001; //< a factor, is it emperical?
	float l2 = L2 * damping * damping;

	return 
		op.A 
		* exp(-1.0f / (k_length2 * L2)) / k_length4  
		* k_dot_w2 
		* exp(-k_length2 * l2); //< again the factor
}

static float UniformRand() {
	return (float)rand() / RAND_MAX;
}

// Box¨CMuller
// https://blog.csdn.net/weixin_41793877/article/details/84700875
static Complex GaussRandComplex() {
	float x1, x2, w;
	do {
		x1 = 2.f * UniformRand() - 1.f;
		x2 = 2.f * UniformRand() - 1.f;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.f);

	w = sqrt((-2.f * log(w)) / w);
	return Complex(x1 * w, x2 * w);
}

// i.e. sqrt(phillips(x,y) / 2) * nounce
static Complex H_Tilde_0(const OceanParameters& op, int n_prime, int m_prime) {
	return GaussRandComplex() * sqrt(Phillips(op, n_prime, m_prime) / 2.0f);
}

static void PrecomputeOcean(OceanParameters& op) {
	OceanParameters::Data* pdata = new OceanParameters::Data;
	auto& data = *pdata;

	int Nplus1 = op.N + 1;
	int N = op.N;
	int length = op.length;

	data.shader_data.clear();
	size_t expected_size = static_cast<size_t>(op.N + 1) * static_cast<size_t>(op.N + 1);
	data.shader_data.resize(expected_size);

	data.precompute.clear();
	data.precompute.resize(expected_size);

	for (int m_prime = 0; m_prime < Nplus1; m_prime++) {
		for (int n_prime = 0; n_prime < Nplus1; n_prime++) {
			int index = m_prime * Nplus1 + n_prime;
			auto htilde0 = H_Tilde_0(op, n_prime, m_prime);
			auto htilde0_conj = std::conj(H_Tilde_0(op, -n_prime, -m_prime));

			auto& sdata = data.shader_data[index];
			auto& cdata = data.precompute[index];

			sdata.htilde0.x = htilde0.real();
			sdata.htilde0.y = htilde0.imag();

			cdata.htilde0_conj.x = htilde0_conj.real();
			cdata.htilde0_conj.y = htilde0_conj.imag();

			cdata.orgin.x = (n_prime - N / 2.f) * length / N;
			cdata.orgin.y = 0.f;
			cdata.orgin.z = (m_prime - N / 2.f) * length / N;
			
			sdata.normal.x = 0.0f;
			sdata.normal.y = 1.0f;
			sdata.normal.z = 0.0f;
		}
	}

	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			int index = m_prime * Nplus1 + n_prime;

			auto& indices = data.indices;
			indices.multi_push_back(index, index + Nplus1, index + Nplus1 + 1);
			indices.multi_push_back(index, index + Nplus1 + 1, index + 1);
		}
	}


}

// h0 * exp(iwt) +h0' * exp(-iwt)
static Complex H_Tilde(OceanParameters& op, float t, int n_prime, int m_prime) {
	int index = m_prime * (op.N + 1) + n_prime;

	auto& data = *op.data;
	auto& sdata = data.shader_data[index];
	auto& cdata = data.precompute[index];

	Complex htilde0(sdata.htilde0.x, sdata.htilde0.y);
	Complex htilde0_conj(cdata.htilde0_conj.x, cdata.htilde0_conj.y);

	//TODO: Precompute omega
	float omega_t = PreComputeOmega(op, n_prime, m_prime) * t;

	float cos_omega = cos(omega_t);
	float sin_omega = sin(omega_t);

	Complex c0(cos_omega, sin_omega);
	Complex c1(cos_omega, -sin_omega);
	
	return htilde0 * c0 + htilde0_conj * c1;
}

static std::tuple<Complex, glm::vec2, glm::vec3> 
H_D_And_N(OceanParameters& op, glm::vec2 x, float t) {

	Complex h(0.0f, 0.0f);
	glm::vec2 D(0.0f, 0.0f);
	glm::vec3 n(0.0f, 0.0f, 0.0f);

	Complex res;
	
	int N = op.N;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		float kz = op.Kz(m_prime);
		for (int n_prime = 0; n_prime < N; n_prime++) {
			float kx = op.Kx(n_prime);
			glm::vec2 k(kx, kz);

			float k_length = k.length();
			float k_dot_x = glm::dot(k, x);

			Complex c(cos(k_dot_x), sin(k_dot_x));
			Complex htilde_c = H_Tilde(op, t, n_prime, m_prime) * c;
			h = h + htilde_c;

			n = n + glm::vec3(-kx * htilde_c.imag(), 0.0f, -kz * htilde_c.imag());
			if (k_length < 0.000001) continue;

			D = D + glm::vec2(kx / k_length * htilde_c.imag(), kz / k_length * htilde_c.imag());
		}
	}

	n = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) - n);
	return std::tuple(h, D, n);
}

void OceanSystem::Data::MakeSpace(OceanParameters& op)
{
	size_t expected_size = static_cast<size_t>(op.N) * static_cast<size_t>(op.N);

	if (h_tilde.size() < expected_size) {
		h_tilde.resize(expected_size);
	}

	if (h_tilde_slopex.size() < expected_size) {
		h_tilde_slopex.resize(expected_size);
	}

	if (h_tilde_slopez.size() < expected_size) {
		h_tilde_slopez.resize(expected_size);
	}

	if (h_tilde_dx.size() < expected_size) {
		h_tilde_dx.resize(expected_size);
	}

	if (h_tilde_dz.size() < expected_size) {
		h_tilde_dz.resize(expected_size);
	}
}

void OceanSystem::Data::EvaluateWavesFFT(OceanParameters& op, float t) {
	int N = op.N;

	MakeSpace(op);

	for (int m_prime = 0; m_prime < N; m_prime++) {
		float kz = op.Kz(m_prime);
		for (int n_prime = 0; n_prime < N; n_prime++) {
			float kx = op.Kx(n_prime);

			float len = sqrt(kx * kx + kz * kz);
			int index = m_prime * N + n_prime;


			h_tilde[index] = H_Tilde(op, t, n_prime, m_prime);
			h_tilde_slopex[index] = h_tilde[index] * Complex(0, kx);
			h_tilde_slopez[index] = h_tilde[index] * Complex(0, kz);

			if (len < 0.000001f) {
				h_tilde_dx[index] = Complex(0.0f, 0.0f);
				h_tilde_dz[index] = Complex(0.0f, 0.0f);
			}
			else {
				h_tilde_dx[index] = h_tilde[index] * Complex(0, -kx / len);
				h_tilde_dz[index] = h_tilde[index] * Complex(0, -kz / len);
			}
		}
	}

	int sign;
	float signs[] = { 1.0f, -1.0f };
	glm::vec3 n;
	size_t Nplus1 = static_cast<size_t>(N) + 1;
	float lambda = -1.0f;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			size_t index = m_prime * N + n_prime;		// index into h_tilde..
			size_t index1 = m_prime * Nplus1 + n_prime;	// index into vertices

			sign = signs[(n_prime + m_prime) & 1];

			h_tilde[index] *= sign;
			h_tilde_dx[index] *= sign;
			h_tilde_dz[index] *= sign;

			auto& vertex = op.data->shader_data[index1].vertex;
			auto& origin = op.data->precompute[index1].orgin;
			auto& normal = op.data->shader_data[index1].normal;

			// height
			vertex.y = h_tilde[index].real();
			// displacement
			vertex.x = origin.x + h_tilde_dx[index].real() * lambda;
			vertex.z = origin.z + h_tilde_dz[index].real() * lambda;

			normal = glm::normalize(glm::vec3(
				0.0f - h_tilde_slopex[index].real(),
				1.0f,
				0.0f - h_tilde_slopez[index].real()
			));

			if (n_prime == 0 && m_prime == 0) {
				auto& v = op.data->shader_data[index1 + N + Nplus1 * N];
				auto& p = op.data->precompute[index1 + N + Nplus1 * N];
				v.vertex.y = h_tilde[index].real();
				
				v.vertex.z = p.orgin.z + h_tilde_dz[index].real() * lambda;
				v.vertex.x = p.orgin.x + h_tilde_dx[index].real() * lambda;
				
				v.normal = n;
			}
			if (n_prime == 0) {
				auto& v = op.data->shader_data[index1 + N];
				auto& p = op.data->precompute[index1 + N];
				v.vertex.y = h_tilde[index].real();

				v.vertex.x = p.orgin.x + h_tilde_dx[index].real() * lambda;
				v.vertex.z = p.orgin.z + h_tilde_dz[index].real() * lambda;
				
				v.normal = n;
			}

			if (m_prime == 0) {
				auto& v = op.data->shader_data[index1 + Nplus1 * N];
				auto& p = op.data->precompute[index1 + Nplus1 * N];
				v.vertex.y = h_tilde[index].real();

				v.vertex.x = p.orgin.x + h_tilde_dx[index].real() * lambda;
				v.vertex.z = p.orgin.z + h_tilde_dz[index].real() * lambda;

				v.normal = n;
			}
		}
	}



}





}