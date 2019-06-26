#if 0
#include <chrono>
#include <iostream>
#include <vector>

#include "containers/vector.h"
#include "memory/slot-allocator.h"

struct XX
{
	int x;
	char b;
	~XX() {}
};

#include "core/entity-manager.h"
void benchmark_vector();
int main()
{
	using namespace nabla;
	benchmark_vector();
	EntityManager em(3);
	auto a = em.Create();
	auto b = em.Create();
	auto c = em.Create();
	em.Destroy(c);
	em.Destroy(a);
	c = em.Create();
	a = em.Create();
	auto d = em.Create();

	getchar();
	
}

void benchmark_vector()
{
	using namespace nabla;
	int n = 10000000;
	{
		std::vector<XX> s;
		auto beg = std::chrono::steady_clock::now();
		// s.resize(n);
		for (int i = 0; i < n; ++i) {
			s.push_back(XX{});
		}
		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count() << std::endl;
	}

	{
		std::allocator<XX> alloc;
		Vector<XX> s(&alloc);
		auto beg = std::chrono::steady_clock::now();
		// s.resize(n);
		for (int i = 0; i < n; ++i) {
			s.push_back(XX{});
		}
		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count() << std::endl;
		constexpr int aa = std::is_trivially_destructible<XX>::value;

	}

	{
		SlotAllocator alloc("Page Alloc A");
		SlotAllocatorProxy<XX> slot(alloc);
		Vector<XX, SlotAllocatorProxy<XX>> /*SlotAllocatorProxy<XX>>*/ s(&slot);
		auto beg = std::chrono::steady_clock::now();
		// s.resize(n);
		for (int i = 0; i < n; ++i) {
			s.push_back(XX{});
		}
		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count() << std::endl;
		constexpr int aa = std::is_trivially_destructible<XX>::value;
	}
}
#endif