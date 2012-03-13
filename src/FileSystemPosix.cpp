#include "libs.h"
#include "FileSystem.h"
#include <cassert>
#include <algorithm>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

namespace FileSystem {

	static std::string absolute_path(const std::string &path) {
		if (!path.empty() && path[0] == '/') { return path; }
		else {
			const size_t bufsize = 512;
			ScopedMalloc<char> buf(std::malloc(bufsize));
			char *cwd = getcwd(buf.Get(), bufsize);
			if (!cwd) {
				fprintf(stderr, "failed to get current working directory\n");
				abort();
			}

			std::string abspath;
			if (cwd) abspath = cwd;
			abspath += '/';
			abspath += path;
			return abspath;
		}
	}

	static std::string FindUserDir()
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

	std::string GetUserDir(const char *subdir)
	{
		static const std::string user_path = FindUserDir();
		if (subdir)
			return JoinPathBelow(user_path, subdir);
		else
			return user_path;
	}

	std::string GetDataDir(const char *subdir)
	{
		static const std::string data_path = absolute_path(std::string(PIONEER_DATA_DIR));
		if (subdir)
			return JoinPathBelow(data_path, subdir);
		else
			return data_path;
	}

	FileSourceFS::FileSourceFS(const std::string &root):
		FileSource(absolute_path(root)) {}

	FileSourceFS::~FileSourceFS() {}

	FileInfo FileSourceFS::Lookup(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		struct stat statinfo;
		FileInfo::FileType ty;
		if (stat(fullpath.c_str(), &statinfo) == 0) {
			if (S_ISREG(statinfo.st_mode)) {
				ty = FileInfo::FT_FILE;
			} else if (S_ISDIR(statinfo.st_mode)) {
				ty = FileInfo::FT_DIR;
			} else {
				ty = FileInfo::FT_SPECIAL;
			}
		} else {
			ty = FileInfo::FT_NON_EXISTENT;
		}
		return MakeFileInfo(path, ty);
	}

	RefCountedPtr<FileData> FileSourceFS::ReadFile(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		FILE *fl = fopen(fullpath.c_str(), "rb");
		if (!fl) {
			return RefCountedPtr<FileData>(0);
		} else {
			fseek(fl, 0, SEEK_END);
			long sz = ftell(fl);
			fseek(fl, 0, SEEK_SET);
			char *data = reinterpret_cast<char*>(std::malloc(sz));
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
		const std::string fulldirpath = JoinPathBelow(GetRoot(), dirpath);
		DIR *dir = opendir(fulldirpath.c_str());
		if (!dir) { return false; }
		struct dirent *entry;

		const size_t output_head_size = output.size();

		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0) continue;
			if (strcmp(entry->d_name, "..") == 0) continue;

			const std::string fullpath = JoinPath(fulldirpath, entry->d_name);

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

			output.push_back(MakeFileInfo(fullpath.substr(GetRoot().size() + 1), ty));
		}

		closedir(dir);

		std::sort(output.begin() + output_head_size, output.end());
		return true;
	}

	static bool make_directory_raw(const std::string &path)
	{
		// allow multiple tries (to build parent directories)
		while (true) {
			if (mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
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
						if (pos != std::string::npos) {
							const std::string dirname = path.substr(0, pos-1);
							if (dirname.empty() || !make_directory_raw(dirname)) {
								return false;
							}
						} else
							return false;
					}
				default: return false;
				}
			} else {
				return true;
			}
		}
	}

	bool FileSourceFS::MakeDirectory(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		return make_directory_raw(fullpath);
	}
}
