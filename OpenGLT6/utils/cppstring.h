#ifndef _NABLA_UITLS_CPP_STRING_H_
#define _NABLA_UITLS_CPP_STRING_H_
#include <string>
#include "containers/vector.h"

namespace nabla {
inline Vector<std::string> Explode(const std::string& s, const char& c)
{
	std::string buff{ "" };
	Vector<std::string> v;

	for (auto n : s)
	{
		if (n != c)
		{
			buff += n;
		}
		else if (n == c && buff != "") {
			v.push_back(std::move(buff));
			buff.clear();
		}
	}
	if (buff != "")
	{
		v.push_back(std::move(buff));
	}

	return v;
}
}

#endif // !_NABLA_UITLS_CPP_STRING_H_

