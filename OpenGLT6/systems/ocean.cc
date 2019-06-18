#include "ocean.h"
#include "ocean/fft.h"

#include "editor/gui.h"
#include "core/renderer.h"

#include <complex>
#include <thread>
#include <future>

namespace nabla {
using namespace renderer;
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
	MeshHandle hmesh;
	FFT* fft = nullptr;
};

struct OceanSystem::Data {
	Vector<Complex> h_tilde;
	Vector<Complex> h_tilde_slopex;
	Vector<Complex> h_tilde_slopez;
	Vector<Complex> h_tilde_dx;
	Vector<Complex> h_tilde_dz;

	Vector<Complex> buf_h_tilde;
	Vector<Complex> buf_h_tilde_slopex;
	Vector<Complex> buf_h_tilde_slopez;
	Vector<Complex> buf_h_tilde_dx;
	Vector<Complex> buf_h_tilde_dz;


	OceanParameters* active_ocean;
	ShaderHandle hshader;
	MaterialHandle hproj;
	MaterialHandle hview;
	MaterialHandle hmodel;
	MaterialHandle hlight_pos;
	MaterialHandle hfog_decay;
	MaterialHandle water_texture; //< TODO

	float total_cnt_ = 1.0f;
	int cnt = 0;

	void MakeSpace(OceanParameters& op);

	void EvaluateWavesFFT(OceanParameters& op, float t);
};

float OceanParameters::Kx(int n_prime) const
{
	return PI * (2.f * n_prime - N) / length;
}

float OceanParameters::Kz(int m_prime) const
{
	return PI * (2.f * m_prime - N) / length;
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
	OceanParameters::Data* pdata = op.data;

	if (op.data == nullptr) {
		op.data = pdata = new OceanParameters::Data;
	}
	
	auto& data = *pdata;

	int Nplus1 = op.N + 1;
	int N = op.N;
	int length = op.length;

	data.shader_data.clear();
	size_t expected_size = Nplus1 * Nplus1;
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
			cdata.orgin.y = -2.f;
			cdata.orgin.z = (m_prime - N / 2.f) * length / N;
			
			sdata.normal.x = 0.0f;
			sdata.normal.y = 1.0f;
			sdata.normal.z = 0.0f;

			sdata.vertex = cdata.orgin;
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



void OceanSystem::Data::MakeSpace(OceanParameters& op)
{
	size_t N = op.N;
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

	if (buf_h_tilde.size() < N) {
		buf_h_tilde.resize(N);
	}

	if (buf_h_tilde_slopex.size() < N) {
		buf_h_tilde_slopex.resize(N);
	}

	if (buf_h_tilde_slopez.size() < N) {
		buf_h_tilde_slopez.resize(N);
	}

	if (buf_h_tilde_dx.size() < N) {
		buf_h_tilde_dx.resize(N);
	}

	if (buf_h_tilde_dz.size() < N) {
		buf_h_tilde_dz.resize(N);
	}
}

void OceanSystem::Data::EvaluateWavesFFT(OceanParameters& op, float t) {
	size_t N = op.N;

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
	size_t Nplus1 = N + 1;
	float lambda = -1.0f;

	auto& fft = *op.data->fft;

	auto Compute = [this, &fft, N] (Vector<Complex>& target, Vector<Complex>&buf) {
		return std::async([this, &fft, N, &target, &buf] {
			for (int m_prime = 0; m_prime < N; ++m_prime) {
				fft.fft(target.begin(), buf.begin(),  1, m_prime * N);
			}
			for (int n_prime = 0; n_prime < N; ++n_prime) {
				fft.fft(target.begin(), buf.begin(), N, n_prime);
			}
		});
	};
	
	auto res1 = Compute(h_tilde, buf_h_tilde);
	auto res2 = Compute(h_tilde_slopex, buf_h_tilde_slopex);
	auto res3 = Compute(h_tilde_slopez, buf_h_tilde_slopez);
	auto res4 = Compute(h_tilde_dx, buf_h_tilde_dx);
	auto res5 = Compute(h_tilde_dz, buf_h_tilde_dz);
	
	res1.wait();
	res2.wait();
	res3.wait();
	res4.wait();
	res5.wait();


	//for (int m_prime = 0; m_prime < N; ++m_prime) {
	//	fft.fft(h_tilde.begin(), 1, m_prime * N);
	//	fft.fft(h_tilde_slopex.begin(), 1, m_prime * N);
	//	fft.fft(h_tilde_slopez.begin(), 1, m_prime * N);
	//	fft.fft(h_tilde_dx.begin(), 1, m_prime * N);
	//	fft.fft(h_tilde_dz.begin(), 1, m_prime * N);
	//}
	//
	//for (int n_prime = 0; n_prime < N; ++n_prime) {
	//	fft.fft(h_tilde.begin(), N, n_prime);
	//	fft.fft(h_tilde_slopex.begin(), N, n_prime);
	//	fft.fft(h_tilde_slopez.begin(), N, n_prime);
	//	fft.fft(h_tilde_dx.begin(), N, n_prime);
	//	fft.fft(h_tilde_dz.begin(), N, n_prime);
	//}

#pragma loop(hint_parallel(8))
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
			vertex.y = origin.y + h_tilde[index].real();
			// displacement
			vertex.x = origin.x + h_tilde_dx[index].real() * lambda;
			vertex.z = origin.z + h_tilde_dz[index].real() * lambda;

			h_tilde_slopex[index] *= sign;
			h_tilde_slopez[index] *= sign;
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
				
				v.normal = normal;
			}
			if (n_prime == 0) {
				auto& v = op.data->shader_data[index1 + N];
				auto& p = op.data->precompute[index1 + N];
				v.vertex.y = p.orgin.y + h_tilde[index].real();

				v.vertex.x = p.orgin.x + h_tilde_dx[index].real() * lambda;
				v.vertex.z = p.orgin.z + h_tilde_dz[index].real() * lambda;
				
				v.normal = normal;
			}

			if (m_prime == 0) {
				auto& v = op.data->shader_data[index1 + Nplus1 * N];
				auto& p = op.data->precompute[index1 + Nplus1 * N];
				v.vertex.y = p.orgin.y + h_tilde[index].real();

				v.vertex.x = p.orgin.x + h_tilde_dx[index].real() * lambda;
				v.vertex.z = p.orgin.z + h_tilde_dz[index].real() * lambda;

				v.normal = normal;
			}
		}
	}
}


void OceanSystem::Initialize(SystemContext&)
{
	data_ = new Data;
	auto& d = *data_;

	d.hshader = NewShader({
		"nabla/shaders/ocean.vs",
		"nabla/shaders/ocean.fs"
		});

	d.hproj = NewUniform(d.hshader, "projection", MaterialType::kMat4);
	d.hview = NewUniform(d.hshader, "view", MaterialType::kMat4);
	d.hmodel = NewUniform(d.hshader, "model", MaterialType::kMat4);
	d.hlight_pos = NewUniform(d.hshader, "light_position", MaterialType::kVec3);
	d.hfog_decay = NewUniform(d.hshader, "fog_decay", MaterialType::kFloat);

	d.active_ocean = new OceanParameters;
	auto& p = *d.active_ocean;
	PrecomputeOcean(p);
	{
		Vector<LayoutInfo> layouts;
		layouts.push_back(LayoutInfo{
			0,
			3,
			LayoutInfo::kFloat,
			MaterialType::kVec3,
			false,
			sizeof(ShaderData),
			0u
			});
		
		layouts.push_back(LayoutInfo{
			1,
			3,
			LayoutInfo::kFloat,
			MaterialType::kVec3,
			false,
			sizeof(ShaderData),
			sizeof(glm::vec3)
			});

		layouts.push_back(LayoutInfo{
			2,
			2,
			LayoutInfo::kFloat,
			MaterialType::kVec2,
			false,
			sizeof(ShaderData),
			sizeof(glm::vec3) * 2
			});
		
		p.data->hmesh = NewMesh(
			{ p.data->shader_data.begin(), p.data->shader_data.size_by_bytes() },
			{ p.data->indices.begin(), p.data->indices.size_by_bytes() },
			layouts);
	}
	
	p.data->fft = new FFT;
	p.data->fft->Init(p.N);
}

void OceanSystem::OnGui(const Vector<Entity>& actives)
{
	auto& o = *data_->active_ocean;
	auto& d = *data_;
	if (ImGui::CollapsingHeader("Ocean")) {
		ImGui::Text("Avg render frames: %.1f", d.total_cnt_);
		ImGui::DragFloat("Length", &o.length);
		ImGui::DragFloat3("Light Pos", &o.light_pos.x);
		ImGui::DragFloat("Fog Decay", &o.fog_decay);
		ImGui::Text("Must Recompute:");
		ImGui::DragFloat2("Wind", &o.w.x);
	}
}

void OceanSystem::Update(Clock& clock)
{
	auto& d = *data_;
	auto& o = *d.active_ocean;
	static std::future<void> promise;

	if (!promise.valid()) {
		promise = std::async([this, &d, &o, &clock] {
			d.EvaluateWavesFFT(o, clock.Time());
		});
		d.total_cnt_ = 0.8 * d.cnt + 0.2 * d.total_cnt_;
		d.cnt = 0;
	}
	else if (promise.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
		promise.get();
		glBindBuffer(GL_ARRAY_BUFFER, OpenHandle(o.data->hmesh).vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, o.data->shader_data.size_by_bytes(), o.data->shader_data.begin());
	}
	++d.cnt;

	{
		ScopedState scope(RenderPass::kPostProc);
		glm::mat4 model(1.0f);
		UseShader(d.hshader);
		SetUniform(d.hproj, GetGlobalProjectionMatrix());
		SetUniform(d.hview, GetGlobalViewMatrix());

		SetUniform(d.hlight_pos, o.light_pos);
		SetUniform(d.hfog_decay, o.fog_decay);
		for (int j = 0; j < 20; j++) {
			for (int i = 0; i < 20; i++) {
				model = glm::scale(glm::mat4(1.0f), glm::vec3(5.f, 5.f, 5.f));
				model = glm::translate(model, glm::vec3(o.length * i, 0, o.length * -j));
				SetUniform(d.hmodel, model);
				DrawMesh(o.data->hmesh);
			}
		}
	}
	
}


// static std::tuple<Complex, glm::vec2, glm::vec3>
// H_D_And_N(OceanParameters& op, glm::vec2 x, float t) {
// 
// 	Complex h(0.0f, 0.0f);
// 	glm::vec2 D(0.0f, 0.0f);
// 	glm::vec3 n(0.0f, 0.0f, 0.0f);
// 
// 	Complex res;
// 
// 	int N = op.N;
// 
// 	for (int m_prime = 0; m_prime < N; m_prime++) {
// 		float kz = op.Kz(m_prime);
// 		for (int n_prime = 0; n_prime < N; n_prime++) {
// 			float kx = op.Kx(n_prime);
// 			glm::vec2 k(kx, kz);
// 
// 			float k_length = k.length();
// 			float k_dot_x = glm::dot(k, x);
// 
// 			Complex c(cos(k_dot_x), sin(k_dot_x));
// 			Complex htilde_c = H_Tilde(op, t, n_prime, m_prime) * c;
// 			h = h + htilde_c;
// 
// 			n = n + glm::vec3(-kx * htilde_c.imag(), 0.0f, -kz * htilde_c.imag());
// 			if (k_length < 0.000001) continue;
// 
// 			D = D + glm::vec2(kx / k_length * htilde_c.imag(), kz / k_length * htilde_c.imag());
// 		}
// 	}
// 
// 	n = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) - n);
// 	return std::tuple(h, D, n);
// }
}