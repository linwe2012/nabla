#ifndef _NABLA_CONTAINERS_VECTOR_H_
#define _NABLA_CONTAINERS_VECTOR_H_

#pragma warning (push, 0)
#include <memory>
#include <vector>
#pragma (pop)

#include "memory/memmanip.h"
#include "logger.h"


namespace nabla {



/** This vector is an modified version, conforming part of STL standard
 It must accept a pointer to allocator, and will not be responsible for destroying allocator
 This is NOT thread safe
 @note This vector does NOT owns allocator and it will NEVER destroy it's allocator
*/
template<typename T, typename Alloc = std::allocator<T>>
class STLVectorEx {
public:
	using value_type             = T;
	using allocator_type         = Alloc;
	using size_type              = std::size_t;
	using difference_type        = std::ptrdiff_t;
	using reference              = value_type &;
	using pointer                = T *;          // std::allocator_traits<Alloc>::pointer;
	using const_pointer          = const T*;     // std::allocator_traits<Alloc>::const_pointer;
	using iterator               = pointer;
	using const_iterator         = const_pointer;
	
	reference operator[](size_t pos) {
		NA_ASSERT(pos < size(), "accessing elements out of bound");
		return begin_[pos];
	}

	const reference operator[](size_t pos) const {
		NA_ASSERT(pos < size(), "accessing elements out of bound");
		return begin_[pos];
	}

	STLVectorEx() noexcept : begin_(nullptr), end_(nullptr), end_of_storage_(nullptr), alloc_(default_alloc) {}

	STLVectorEx(STLVectorEx&& rhs) noexcept : STLVectorEx() {
		swap(rhs);
	}

	STLVectorEx(Alloc* alloc) noexcept : begin_(nullptr), end_(nullptr), end_of_storage_(nullptr), alloc_(alloc){}
	
	STLVectorEx(std::initializer_list<T> init) : STLVectorEx() {
		insert(begin(), init.begin(), init.end());
	}

	STLVectorEx(const STLVectorEx& rhs) 
		: begin_(nullptr), end_(nullptr), end_of_storage_(nullptr)
	{
		alloc_ = rhs.alloc_;
		reserve(rhs.capacity());
		end_ = begin_ + rhs.size();
		nabla::detail::SelectiveCopy(begin_, rhs.begin(), rhs.size());
		end_of_storage_ = begin_ + rhs.capacity();
	}

	// Capacity
	bool   empty()    const { return begin_ == begin_; }
	size_t size()     const { return end_ - begin_; }
	size_t size_by_bytes() const { return size() * sizeof(T); }
	size_t capacity() const { return end_of_storage_ - begin_; }

	iterator begin() { return begin_; }
	iterator end()   { return end_; }
	const_iterator begin() const { return begin_; }
	const_iterator end()   const { return end_; }


	void swap(STLVectorEx& rhs) noexcept {
		using std::swap;

		swap(rhs.begin_, begin_);
		swap(rhs.end_, end_);
		swap(rhs.end_of_storage_, end_of_storage_);
		swap(alloc_, alloc_);
	}


	const STLVectorEx& operator=(const STLVectorEx& rhs) {
		if (&rhs == this) {
			return *this;
		}

		this->~STLVectorEx();

		// simply call the copy ctor
		::new(this) STLVectorEx(rhs);

		return *this;
	}

	bool operator==(const STLVectorEx& rhs) const noexcept {
		return this == &rhs;
	}

	void emplace_back(T&& val) {
		ensure_space();
		::new(end_) T(val);
		++end_;
	}

	void push_back(T&& val) {
		emplace_back(std::forward<T>(val));
	}

	void push_back(const T& val) {
		emplace_back(std::move(T(val)));
	}

	template<typename T, typename... Args>
	void multi_push_back(const T& val, Args... args) {
		push_back(val);
		multi_push_back(args...);
	}

	template<typename T>
	void multi_push_back(const T& val) {
		push_back(val);
	}

	// directly construct, avoid move/copy-destruct
	template<typename... Args>
	void construct_back(Args... args) {
		ensure_space();
		::new(end_) T{ args... };
		++end_;
	}

	// insert a value right at the
	iterator insert(iterator position, const value_type& val) {
		NA_ASSERT(position > begin_ && position <= end_, "iterator is out of range");
		if (end_of_storage_ == end_) {
			int new_size = size() * 2;
			T* new_begin = static_cast<T*>(alloc_->allocate(new_size));
			T* new_end = new_begin + size();
			iterator new_pos = new_begin + (position - begin_);
			memcpy(new_begin, begin_, (position - begin_) * sizeof(T));
			memcpy(new_pos + 1, position, (end_ - position) * sizeof(T));
			alloc_->deallocate(begin_, capacity());
			::new(position) T(val);
			begin_ = new_begin;
			end_ = new_end;
			++end_;
			end_of_storage_ = begin_ + new_size;
			return new_pos;
		}
		
		memcpy(position + 1, position, (end_ - position) * sizeof(T));
		::new(position) T(val);
		++end_;
		return position;
	}

	void insert(iterator position, const_iterator first, const_iterator last) {
		NA_ASSERT(position >= begin_ && position <= end_, "iterator is out of range");
		size_t size_required = last - first;
		if (size_required > capacity() - size()) {
			size_t new_cap = (capacity() == 0) ? 1 : capacity();
			while (new_cap < size_required + size())
			{
				new_cap *= 2;
			}
			T* new_begin = static_cast<T*>(alloc_->allocate(new_cap));
			T* new_end = new_begin + size();
			iterator new_pos = new_begin + (position - begin_);
			memcpy(new_begin, begin_, (position - begin_) * sizeof(T));
			if (position != nullptr) {
				memcpy(new_pos + size_required, position, (end_ - position) * sizeof(T));
			}
			nabla::detail::SelectiveCopy<T>(new_pos, first, size_required);
			if (begin() != nullptr) {
				alloc_->deallocate(begin_, capacity());
			}
			begin_ = new_begin;
			end_ = new_end + size_required;
			end_of_storage_ = begin_ + new_cap;
			return;
		}
		memcpy(position + size_required, position, (end_ - position) * sizeof(T));
		nabla::detail::SelectiveCopy<T>(position, first, size_required);
		end_ += size_required;
	}

	void reserve(size_t new_size) {
		NA_ASSERT(new_size >= size(), "Unable to shrink storage");
		if (size() == 0 && new_size == 0) {
			alloc_->deallocate(begin_, capacity());
			begin_ = nullptr;
			end_ = nullptr;
			end_of_storage_ = nullptr;
			return;
		}

		T* new_begin = static_cast<T*>(alloc_->allocate(new_size));
		T* new_end = new_begin + size();
		if (begin_ != nullptr) {
			memcpy(new_begin, begin_, size() * sizeof(T));
			alloc_->deallocate(begin_, capacity());
		}
		begin_ = new_begin;
		end_ = new_end;
		end_of_storage_ = begin_ + new_size;
	}

	// make sure size is at least `count`, and the final size might be bigger
	size_t size_at_least(size_t count, const T& value) {
		if (count < size()) {
			return size();
		}

		size_t cap = (capacity() == 0) ? 1 : capacity();
		while (count >= cap) {
			cap *= 2;
		}

		resize(cap, value);
		return cap;
	}

	void resize(size_t count) {
		T value;
		resize(count, value);
	}

	void resize(size_t count, const T& value) {
		if (count <= size()) return;
		if (count > capacity()) {
			reserve(count);
		}
		size_t extra = count - size();
		std::fill(end_, end_ + extra, value);
		end_ += extra;
	}

	void clear() {
		selective_clear<std::is_trivially_destructible<T>::value>();
	}

	reference back() {
		NA_ASSERT(size() > 0, "size of vertex must be larger than 0");
		return *(end_ - 1);
	}

	const reference back() const {
		NA_ASSERT(size() > 0, "size of vertex must be larger than 0");
		return *(end_ - 1);
	}

	void pop_back() {
		NA_ASSERT(size() > 0, "size of vertex must be larger than 0");
		detail::SelectiveDestroy<T>(--end_);
	}

	/** ignore the last element w/o destroying it */
	void ignore_last() {
		if (size()) {
			--end_;
		}
	}

	/** build the vector on the specified memory
	@param data pointer to the data wishing to wrap with a vector
	@note it will not manage (free/reallocate) memory!
	@note this is useful to modify gl arrays
	*/
	void wrap(T* data, size_t _size, size_t _capacity = 0) {
		if (begin_ != nullptr) {
			this->~STLVectorEx();
		}

		begin_ = data;
		end_ = data + _size;
		if (_capacity == 0) {
			end_of_storage_ = data + _size;
		}
		else {
			end_of_storage_ = data + _capacity;
		}
		
		alloc_ = nullptr;
	}

	void unwrap() {
		begin_ = end_ = end_of_storage_ = nullptr;
		alloc_ = default_alloc;
	}

	bool is_wrap() {
		return alloc_ == nullptr;
	}

	void erase(iterator postion) {
		erase(postion, postion + 1);
	}

	void erase(iterator first, iterator last) {
		NA_ASSERT(first < last && 
			first >= begin() && last <= end(), "Erasing element out of range");

		selective_destroy<std::is_trivially_destructible<T>::value>(first, last);

		memmove(first, last, (end() - last) * sizeof(T));
		end_ -= (last - first);
	}

	~STLVectorEx() {
		clear();
		if (alloc_ != nullptr && begin_ != nullptr) {
			alloc_->deallocate(begin_, capacity());
		}
		begin_ = end_ = end_of_storage_ = nullptr;
	}

	
private:
	/** ensure there is space for at least one element */
	void ensure_space() {
		if (end_of_storage_ == end_) {
			if (size() == 0) reserve(2);
			else reserve(size() * 2);
		}
	}

	template<bool istrivial>
	void selective_clear() {
		static_assert(false, "This should never be called");
	}

	/** for trivailly destructable types incl. `int float` etc. */
	template<>
	void selective_clear< true >() {
		end_ = begin_;
	}

	/** not trivially destructable, we have to cal it's detor
	* normally has pointer to release or destructor is defined 
	*/
	template<>
	void selective_clear< false >() {
		while (end_ != begin_)
		{
			--end_;
			end_->~T();
		}
	}

	template<bool istrivial>
	void selective_destroy(iterator first, iterator last) {
		static_assert(false, "This should never be called");
	}

	template<>
	void selective_destroy< true >(iterator first, iterator last) {
		return;
	}

	template<>
	void selective_destroy< false >(iterator first, iterator last) {
		while (first != last)
		{
			first->~T();
			++first;
		}
	}

	T* begin_;
	T* end_;
	T* end_of_storage_;
	Alloc* alloc_;
	static inline Alloc* default_alloc = new Alloc; // works in c++17
};

// thread safe vector
template<typename T, typename Alloc = ::std::allocator<T>>
using SafeVector = ::std::vector<T, Alloc>;


template<typename T, typename Alloc = ::std::allocator<T>>
using Vector = STLVectorEx<T, Alloc>;

}

#endif // !_NABLA_CONTAINERS_VECTOR_H_
