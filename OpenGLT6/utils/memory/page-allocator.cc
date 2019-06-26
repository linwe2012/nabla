#include "page-allocator.h"
#include "platform-info.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>

static size_t get_page_size()
{
	static_assert(sizeof(size_t) >= sizeof(DWORD), "possible loss of data");

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return size_t(info.dwPageSize);
}

const static size_t virtual_memory_page_size = get_page_size();

static size_t roundUp(size_t size) {
	return (size + virtual_memory_page_size - 1) & ~(virtual_memory_page_size - 1);
}

PageAllocator::PageAllocator(const char* name, size_t size)
	:name_(name)
{
	size = 1 * GB;
	size_t n = 0;
	size_t lim = virtual_memory_page_size;
	while (lim < size)
	{
		lim *= 2;
		++n;
	}
	current_ = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
	// memory_ = VirtualAlloc(NULL, 2 * GB, MEM_COMMIT, PAGE_READWRITE);
	if (current_ == NULL) {
		auto error_code = GetLastError();
		(void)error_code;
		//TODO(L) throw error
	}
	end_ = static_cast<void*>((char *)current_ + size);
}

void* PageAllocator::allocate(size_t size)
{
	size_t true_size = roundUp(size);
	auto res =  VirtualAlloc(current_, true_size, MEM_COMMIT, PAGE_READWRITE);
	current_ = (char*)current_ + true_size;
	return res;
	// return VirtualAlloc(NULL, true_size, MEM_COMMIT, PAGE_READWRITE);
}

void PageAllocator::deallocate(void* p, size_t size)
{
	size_t true_size = roundUp(size);
	current_ = (char*)current_ - true_size;
	VirtualFree(p, true_size, MEM_DECOMMIT);
}







#endif // PLATFORM_WINDOWS
