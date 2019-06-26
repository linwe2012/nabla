#ifndef _NABLA_UTILS_FILESYSTEM_H_
#define _NABLA_UTILS_FILESYSTEM_H_
#include <fstream>

#include <functional>
#include <vector>

// namespace fs = std::experimental::filesystem;

#include <filesystem>
namespace fs = std::filesystem;

namespace nabla {
	/*
class File {
public:
	explicit File(const char* filename) :path_(filename) {}
	explicit File(const fs::path& p) :path_(p) {}
	explicit File(fs::path&& p) :path_(p) {}
	// explicit File(fs::path p) :path_(p) {}

	std::ofstream asOutput();
	std::ifstream asInput();

	bool isDir();
	bool isFile();

	bool isExist();
	static bool isExist(const char* filename);

	const fs::path& path() const { return path_; }
	std::string string() const { return path_.string(); }

	// permissions
	bool canOwnerRead();

	void onChange(std::function<void(File&)> callback);
private:
	fs::path path_;
	std::vector<std::function<void(File&)>> callbacks_;
};


inline std::ofstream File::asOutput() {
	return std::ofstream(path_);
}

inline std::ifstream File::asInput() {
	return std::ifstream(path_);
}

inline bool File::isDir()
{
	return fs::is_directory(path_);
}

inline bool File::isFile()
{
	return fs::is_regular_file(path_);
}

inline bool File::isExist()
{
	return fs::exists(path_);
}

inline bool File::isExist(const char* filename)
{
	return fs::exists(filename);
}*/

void ListenFile(fs::path path, std::function<void(fs::path&)> callback);

}


#endif // !_NABLA_UTILS_FILESYSTEM_H_
