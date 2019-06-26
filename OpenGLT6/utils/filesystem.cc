#include "filesystem.h"
#include <map>
namespace nabla {
	using ListenCB = std::function<void(fs::path&)>;
	std::map<fs::path, std::vector<ListenCB>> gListeners;

void ListenFile(fs::path path, std::function<void(fs::path&)> callback)
{
	gListeners[path].push_back(callback);
}
}