#ifndef _NABLA_MEMEORY_PAGE_ALLOCATOR_H_
#define _NABLA_MEMEORY_PAGE_ALLOCATOR_H_


class PageAllocator {
	enum : size_t {
		KB = 1024, 
		MB = 1024 * KB,
		GB = 1024 * MB
	};
public:
	PageAllocator(const char *name, size_t size = 4 * GB);
	// ~PageAllocator();
	void* allocate(size_t size);
	void deallocate(void* p, size_t size);
	
	const char* name_;
	void* current_;
	void* end_;
};

template<typename T>
class PageAllocProxy {
public:
	PageAllocProxy(PageAllocator& alloc) :alloc_(alloc) {};
	void* allocate(size_t count) { return alloc_.allocate(count * sizeof(T)); };
	void deallocate(void* p, size_t size) {
		alloc_.deallocate(p, size * sizeof(T));
	}

	PageAllocator& alloc_;
};
#endif // !_NABLA_MEMEORY_PAGE_ALLOCATOR_H_
