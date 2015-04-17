// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "FileSystem.h"
#include "utils.h"
#include <cassert>
#include <algorithm>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// on unix this is set from configure
#ifndef PIONEER_DATA_DIR
#define PIONEER_DATA_DIR "data"
#endif

#ifdef _XCODE
#include "CoreFoundation/CoreFoundation.h"
#import <sys/param.h> /* for MAXPATHLEN */
#endif

namespace FileSystem {

	static std::string absolute_path(const std::string &path) {
		if (!path.empty() && path[0] == '/') { return path; }
		else {
			const size_t bufsize = 512;
			std::unique_ptr<char, FreeDeleter> buf(static_cast<char*>(std::malloc(bufsize)));
			char *cwd = getcwd(buf.get(), bufsize);
			if (!cwd) {
				Output("failed to get current working directory\n");
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

    static std::string FindDataDir()
    {
#ifdef _XCODE
        // On OSX, the data directory is located in the resources folder of the application
        // bundle (Contents/Resources). Rather than using cwd (which is the cwd of the app.bundle
        // folder)
        // - This is XCode/App Bundle specific
        std::string path;

        char appbundlepath[MAXPATHLEN];
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)appbundlepath, MAXPATHLEN))
        {
            path = appbundlepath;
            path += '/';
            path += PIONEER_DATA_DIR;
        }
        CFRelease(resourcesURL);
        return path;
#else
        return absolute_path(std::string(PIONEER_DATA_DIR));
#endif
    }

	std::string GetUserDir()
	{
		static const std::string user_path = FindUserDir();
		return user_path;
	}

	std::string GetDataDir()
	{
        static const std::string data_path = FindDataDir();
		return data_path;
	}

	FileSourceFS::FileSourceFS(const std::string &root, bool trusted):
		FileSource(absolute_path(root), trusted) {}

	FileSourceFS::~FileSourceFS() {}

	static FileInfo::FileType interpret_stat(const struct stat &info, Time::DateTime &mtime) {
		FileInfo::FileType ty;
		if (S_ISREG(info.st_mode)) {
			ty = FileInfo::FT_FILE;
		} else if (S_ISDIR(info.st_mode)) {
			ty = FileInfo::FT_DIR;
		} else {
			ty = FileInfo::FT_SPECIAL;
		}

		struct tm timeparts;
		if (localtime_r(&info.st_mtime, &timeparts) != nullptr) {
			mtime = Time::DateTime(
				1900 + timeparts.tm_year, timeparts.tm_mon+1, timeparts.tm_mday,
				timeparts.tm_hour, timeparts.tm_min, timeparts.tm_sec);
		}

		return ty;
	}

	static FileInfo::FileType stat_path(const char *fullpath, Time::DateTime &mtime) {
		struct stat info;
		if (stat(fullpath, &info) == 0) {
			return interpret_stat(info, mtime);
		} else {
			return FileInfo::FT_NON_EXISTENT;
		}
	}

	static FileInfo::FileType stat_fd(int fd, Time::DateTime &mtime) {
		struct stat info;
		if (fstat(fd, &info) == 0) {
			return interpret_stat(info, mtime);
		} else {
			return FileInfo::FT_NON_EXISTENT;
		}
	}

	FileInfo FileSourceFS::Lookup(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		Time::DateTime mtime;
		FileInfo::FileType ty = stat_path(fullpath.c_str(), mtime);
		return MakeFileInfo(path, ty, mtime);
	}

	RefCountedPtr<FileData> FileSourceFS::ReadFile(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		FILE *fl = fopen(fullpath.c_str(), "rb");
		if (!fl) {
			return RefCountedPtr<FileData>(0);
		} else {
			Time::DateTime mtime;
			FileInfo::FileType ty = stat_fd(fileno(fl), mtime);
			assert(ty == FileInfo::FT_FILE);

			fseek(fl, 0, SEEK_END);
			long sz = ftell(fl);
			fseek(fl, 0, SEEK_SET);
			char *data = static_cast<char*>(std::malloc(sz));
			if (!data) {
				// XXX handling memory allocation failure gracefully is too hard right now
				Output("failed when allocating buffer for '%s'\n", fullpath.c_str());
				fclose(fl);
				abort();
			}
			size_t read_size = fread(data, 1, sz, fl);
			if (read_size != size_t(sz)) {
				Output("file '%s' truncated!\n", fullpath.c_str());
				memset(data + read_size, 0xee, sz - read_size);
			}
			fclose(fl);

			return RefCountedPtr<FileData>(new FileDataMalloc(MakeFileInfo(path, ty, mtime), sz, data));
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

			FileInfo::FileType ty;
			Time::DateTime mtime;

			struct stat info;
			if (stat(JoinPath(fulldirpath, entry->d_name).c_str(), &info) == 0) {
				ty = interpret_stat(info, mtime);
			} else {
				ty = FileInfo::FT_NON_EXISTENT;
			}

			output.push_back(MakeFileInfo(JoinPath(dirpath, entry->d_name), ty, mtime));
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

	FILE* FileSourceFS::OpenReadStream(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		return fopen(fullpath.c_str(), "rb");
	}

	FILE* FileSourceFS::OpenWriteStream(const std::string &path, int flags)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		return fopen(fullpath.c_str(), (flags & WRITE_TEXT) ? "w" : "wb");
	}
}
