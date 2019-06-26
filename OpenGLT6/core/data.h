#ifndef _NABLA_CORE_DATA_H_
#define _NABLA_CORE_DATA_H_
#include <memory>
namespace nabla {
namespace data {
#define DATA_TYPES(V) \
	V(Integer, int64_t) \
	

	class IData {

	};


	class Interger : public IData {
	public:
		enum
		{
			kInvalid = UINT64_MAX,
		};

		Interger(int64_t val) : val_(val), max_(kInvalid), min_(kInvalid)
		{}

		Interger(int64_t val, int64_t max, int64_t min) : val_(val), max_(max), min_(min)
		{}

		int64_t Value() { return val_; }

		void set_max(int64_t max) { max_ = max; }
		int64_t Max() { return max_; }
		bool HasMax() { return max_ != kInvalid; }

		void set_min(int64_t min) { max_ = min; }
		int64_t Min() { return min_; }
		bool HasMin() { return min_ != kInvalid; }

	private:
		int64_t val_;
		int64_t max_;
		int64_t min_;
	};


	class Color {

		char red_;
		char green_;
		char blue_;
		char alpha_;
	};

	
	
}
}

#endif // !_NABLA_CORE_DATA_H_
