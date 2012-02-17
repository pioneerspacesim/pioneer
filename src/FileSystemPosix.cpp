#include "libs.h"
#include "FileSystem.h"
#include <cassert>

namespace FileSystem {

	std::string GetUserDir()
	{
		std::string path = getenv("HOME");
		if (!path.empty() && (path[path.size()-1] != '/')) {
			path += '/';
		}

#ifdef __APPLE__
		path += "Library/Application Support/Pioneer";
#else
		path += ".pioneer";
#endif

		return path;
	}

	std::string GetDataDir()
	{
		return std::string(PIONEER_DATA_DIR "/");
	}

	FileSourceFS::FileSourceFS(const std::string &root):
		FileSource(root) {}

	FileSourceFS::~FileSourceFS() {}

	FileInfo FileSourceFS::Lookup(const std::string &path)
	{
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);
	}

	RefCountedPtr<FileData> FileSourceFS::ReadFile(const std::string &path)
	{
		return RefCountedPtr<FileData>();
	}

	void FileSourceFS::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
	{
	}

	void FileSourceFS::MakeDirectory(const std::string &path)
	{
	}
}
