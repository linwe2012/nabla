#ifndef  _NABLA_MEMORY_SLOT_ALLOCATOR_H_
#define _NABLA_MEMORY_SLOT_ALLOCATOR_H_

#include "platform-info.h"
#include <stdint.h>
#include <stdlib.h>

namespace nabla {
class Allocator;
template<int Increament = 10 * 1024 * 1024>
class SlotAllocator {
public:
	SlotAllocator(const char* name)
		:name_(name), end_free_(nullptr), start_free_(nullptr)
	{
		memset(free_list_, 0, sizeof(SlotData*) * kNumFreeLists);
	}

	void* allocate(size_t n) {
		if (n > kMaxBytes) return malloc(n);
		SlotData** slot_location = free_list_ + freelistIndex(n);
		SlotData* result = *slot_location;
		if (result == nullptr) {
			return refill(roundUp(n));
		}
		*slot_location = result->free_list_link;
		return result;
	}

	void deallocate(void* p, size_t n) {
		if (n > kMaxBytes) return free(p);
		SlotData* slot = static_cast<SlotData*>(p);
		SlotData** slot_location = free_list_ + freelistIndex(n);
		slot->free_list_link = *slot_location;
		*slot_location = slot;
	}

	void* refill(size_t extra) {
		size_t num_slots = 20;
		char* chunk = chunk_alloc(extra, &num_slots);
		if (num_slots <= 1) return chunk;
		SlotData** slot_location = free_list_ + freelistIndex(extra);
		SlotData* result = (SlotData*)chunk;
		SlotData* cur_slot, *next_slot = (SlotData*)(chunk + extra);
		for (int i = 1; ; ++i) {
			cur_slot = next_slot;
			next_slot = (SlotData*)((char*)cur_slot + extra);
			if (num_slots - 1 == i) {
				cur_slot->free_list_link = nullptr;
				break;
			}
			else {
				cur_slot->free_list_link = next_slot;
			}
		}
		return result;
		
	}


private:
	
	char* chunk_alloc(size_t sz, size_t* num) {
		size_t bytes_left =start_free_ - end_free_;
		size_t total_bytes = sz * (*num);
		char* result;
		if (bytes_left >= total_bytes) {
			result = start_free_;
			start_free_ += total_bytes;
			return result;
		}
		else if (bytes_left >= sz)
		{
			(*num) = bytes_left / sz;
			total_bytes = sz * (*num);
			result = start_free_;
			start_free_ += total_bytes;
			return result;
		}
		else {
			if (bytes_left > 0) {
				SlotData** slot_location = free_list_ + freelistIndex(bytes_left);
				((SlotData*)start_free_)->free_list_link = *slot_location;
				*slot_location = ((SlotData*)start_free_);
			}
			// we just add 1MB
			start_free_ = (char*)malloc(kIncrement);
			if (start_free_ == nullptr) {
				(*num) = 0;
				return nullptr;
			}
			end_free_ = start_free_ + kIncrement;
			return chunk_alloc(sz, num);
		}
	}
	enum : size_t
	{
		kAlign = 8,
		kMaxBytes = 1024,
		kNumFreeLists = kMaxBytes / kAlign,
		kIncrement = 10 * 1024 * 1024, // add one mb space per step
	};

	union SlotData
	{
		union SlotData* free_list_link;
		char client_data[1];
	};

	static size_t roundUp(size_t bytes) {
		return (bytes + kAlign - 1) & ~(kAlign - 1);
	}

	static size_t freelistIndex(size_t bytes) {
		return (bytes + kAlign - 1) / kAlign - 1;
	}


	// using Slot = char*;
	const char* name_;
	// Allocator* alloc_;
	// constexpr static uint32_t slab_size_ = 4 * 1024;
	// constexpr static uint32_t chunk_size_ = 4 * 1024;
	SlotData* free_list_[kNumFreeLists];
	
	char* end_free_;
	char* start_free_;
};


template <typename T, int Increament = 10 * 1024 * 1024>
class SlotAllocatorProxy {
public:
	SlotAllocatorProxy(SlotAllocator<Increament>& alloc) : alloc_(alloc) {}
	void* allocate(size_t cnt) { return alloc_.allocate(cnt * sizeof(T)); }
	void deallocate(void* p, size_t cnt) { return alloc_.deallocate(p, cnt * sizeof(T)); }
private:
	SlotAllocator<Increament>& alloc_;
};
}


#endif // ! _NABLA_MEMORY_SLOT_ALLOCATOR_H_
