#include "libs.h"
#include "FileSystem.h"
#include <cassert>
#include <algorithm>
#include <cerrno>

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
		return std::string(PIONEER_DATA_DIR);
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

	bool FileSourceFS::ReadDirectory(const std::string &dirpath, std::vector<FileInfo> &output)
	{
		const std::string fulldirpath = GetSourcePath() + "/" + dirpath;
		DIR *dir = opendir(fulldirpath.c_str());
		if (!dir) { return false; }
		struct dirent *entry;

		const size_t output_head_size = output.size();

		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0) continue;
			if (strcmp(entry->d_name, "..") == 0) continue;

			const std::string fullpath = fulldirpath + "/" + entry->d_name;

			FileInfo::FileType ty;
			switch (entry->d_type) {
				case DT_DIR: ty = FileInfo::FT_DIR; break;
				case DT_REG: ty = FileInfo::FT_FILE; break;
				case DT_LNK: case DT_UNKNOWN:
				{
					// if readdir() can't tell us whether we've got a file or directory then we need to stat
					// also stat for links to traverse them
					struct stat statinfo;
					if (stat(fullpath.c_str(), &statinfo) == 0) {
						if (S_ISREG(statinfo.st_mode)) {
							ty = FileInfo::FT_FILE;
						} else if (S_ISDIR(statinfo.st_mode)) {
							ty = FileInfo::FT_DIR;
						} else {
							ty = FileInfo::FT_SPECIAL;
						}
					} else {
						// XXX error out here?
						ty = FileInfo::FT_NON_EXISTENT;
					}
					break;
				}
				default: ty = FileInfo::FT_SPECIAL; break;
			}

			output.push_back(MakeFileInfo(fullpath.substr(GetSourcePath().size() + 1), ty));
		}

		closedir(dir);

		std::sort(output.begin() + output_head_size, output.end());
		return true;
	}

	bool FileSourceFS::MakeDirectory(const std::string &path)
	{
		return MakeDirectory(GetSourcePath() + "/" + path);
	}

	bool FileSourceFS::MakeDirectoryRaw(const std::string &path)
	{
		// allow multiple tries (to build parent directories)
		while (true) {
			if (mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO)) {
				switch (errno) {
				case EEXIST:
					{
						struct stat statinfo;
						stat(path.c_str(), &statinfo);
						return S_ISDIR(statinfo.st_mode);
					}
				case ENOENT:
					{
						size_t pos = path.rfind('/');
						const std::string dirname = path.substr(0, pos-1);
						if (dirname.empty() || !MakeDirectoryRaw(dirname)) {
							return false;
						}
					}
				default: return false;
				}
			}
		}
		return true;
	}
}
