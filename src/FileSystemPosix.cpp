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
		const std::string fullpath = GetSourcePath() + "/" + path;
		FILE *fl = fopen(fullpath.c_str(), "rb");
		if (!fl) {
			return RefCountedPtr<FileData>(0);
		} else {
			fseek(fl, 0, SEEK_END);
			long sz = ftell(fl);
			fseek(fl, 0, SEEK_SET);
			unsigned char *data = reinterpret_cast<unsigned char*>(std::malloc(sz));
			if (!data) {
				// XXX handling memory allocation failure gracefully is too hard right now
				fprintf(stderr, "failed when allocating buffer for '%s'\n", fullpath.c_str());
				fclose(fl);
				abort();
			}
			size_t read_size = fread(data, 1, sz, fl);
			if (read_size != size_t(sz)) {
				fprintf(stderr, "file '%s' truncated!\n", fullpath.c_str());
				memset(data + read_size, 0xee, sz - read_size);
			}
			fclose(fl);
			return RefCountedPtr<FileData>(new FileDataMalloc(MakeFileInfo(path, FileInfo::FT_FILE), sz, data));
		}
	}

	void FileSourceFS::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
	{
	}

	void FileSourceFS::MakeDirectory(const std::string &path)
	{
	}
}
