#ifdef _WIN32

#include "libs.h"
#include "FileSystem.h"
#include "TextUtils.h"
#include <cassert>
#include <algorithm>
#include <cerrno>

// I hate macros. I just hate them. Hate hate hate.
#undef FT_FILE

// MinGW targets NT4 by default. We need to set some higher versions to ensure
// that functions we need are available. Specifically, SHCreateDirectoryExA
// requires Windows 2000 and IE5. We include w32api.h to get the symbolic
// constants for these things.
#ifdef __MINGW32__
#	include <w32api.h>
#	ifdef WINVER
#		undef WINVER
#	endif
#	define WINVER Windows2000
#	define _WIN32_IE IE5
#endif

#ifdef _WIN32
#include <windows.h>
// GetPiUserDir() needs these
#include <shlobj.h>
#include <shlwapi.h>
#endif

namespace FileSystem {

	static std::string absolute_path(const std::string &path) {
		std::wstring wpath = transcode_utf8_to_utf16(path);
		wchar_t buf[MAX_PATH+1];
		DWORD len = GetFullPathNameW(wpath.c_str(), MAX_PATH, buf, 0);
		buf[len] = L'\0';
		return transcode_utf16_to_utf8(buf, len);
	}

	static std::string FindUserDir()
	{
		wchar_t appdata_path[MAX_PATH];
		if (SHGetFolderPathW(0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, appdata_path) != S_OK) {
			fprintf(stderr, "Couldn't get user documents folder path\n");
			exit(-1);
		}

		std::wstring path(appdata_path);
		path += L"/Pioneer";

		if (!PathFileExistsW(path.c_str())) {
			if (SHCreateDirectoryExW(0, path.c_str(), 0) != ERROR_SUCCESS) {
				std::string utf8path = transcode_utf16_to_utf8(path);
				fprintf(stderr, "Couldn't create user game folder '%s'", utf8path.c_str());
				exit(-1);
			}
		}

		return transcode_utf16_to_utf8(path);
	}

	static std::string FindDataDir()
	{
		HMODULE exemodule = GetModuleHandleW(0);
		wchar_t buf[MAX_PATH+1];
		DWORD nchars = GetModuleFileNameW(exemodule, buf, MAX_PATH);
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// I gave you MAX_PATH, what more do you want!?
			abort();
		}
		buf[nchars] = L'\0';
		PathRemoveFileSpecW(buf);
		PathAppendW(buf, L"data");
		return transcode_utf16_to_utf8(buf, wcslen(buf));
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
		static const std::string data_path = FindDataDir();
		if (subdir)
			return JoinPathBelow(data_path, subdir);
		else
			return data_path;
	}

	FileSourceFS::FileSourceFS(const std::string &root):
		FileSource((root == "/") ? "" : absolute_path(root)) {}

	FileSourceFS::~FileSourceFS() {}

	static FileInfo::FileType file_type_for_attributes(DWORD attrs)
	{
		FileInfo::FileType ty;
		if (attrs == INVALID_FILE_ATTRIBUTES)
			ty = FileInfo::FT_NON_EXISTENT;
		else if (attrs & FILE_ATTRIBUTE_DIRECTORY)
			ty = FileInfo::FT_DIR;
		else if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
			/* symlink, junction point, etc; we should perhaps traverse it */
			ty = FileInfo::FT_SPECIAL;
		} else if (attrs & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_SYSTEM)) {
			ty = FileInfo::FT_SPECIAL;
		} else
			ty = FileInfo::FT_FILE;
		return ty;
	}

	FileInfo FileSourceFS::Lookup(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		DWORD attrs = GetFileAttributesW(wfullpath.c_str());
		return MakeFileInfo(path, file_type_for_attributes(attrs));
	}

	RefCountedPtr<FileData> FileSourceFS::ReadFile(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		HANDLE filehandle = CreateFileW(wfullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (filehandle == INVALID_HANDLE_VALUE)
			return RefCountedPtr<FileData>(0);
		else {
			LARGE_INTEGER large_size;
			if (!GetFileSizeEx(filehandle, &large_size)) {
				fprintf(stderr, "failed to get file size for '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}
			size_t size = size_t(large_size.QuadPart);

			char *data = reinterpret_cast<char*>(std::malloc(size));
			if (!data) {
				// XXX handling memory allocation failure gracefully is too hard right now
				fprintf(stderr, "failed when allocating buffer for '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}

			if (size > 0x7FFFFFFFull) {
				fprintf(stderr, "file '%s' is too large (can't currently cope with files > 2GB)\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}

			DWORD read_size;
			BOOL ret = ::ReadFile(filehandle, reinterpret_cast<LPVOID>(data), (DWORD)size, &read_size, 0);
			if (!ret) {
				fprintf(stderr, "error while reading file '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}
			if (size_t(read_size) != size) {
				fprintf(stderr, "file '%s' truncated\n", fullpath.c_str());
				memset(data + read_size, 0xee, size - read_size);
			}

			CloseHandle(filehandle);

			return RefCountedPtr<FileData>(new FileDataMalloc(MakeFileInfo(path, FileInfo::FT_FILE), size, data));
		}
	}

	bool FileSourceFS::ReadDirectory(const std::string &dirpath, std::vector<FileInfo> &output)
	{
		size_t output_head_size = output.size();
		const std::wstring wsearchglob = transcode_utf8_to_utf16(JoinPathBelow(GetRoot(), dirpath)) + L"/*";
		WIN32_FIND_DATAW findinfo;
		HANDLE dirhandle = FindFirstFileW(wsearchglob.c_str(), &findinfo);
		DWORD err;

		if (dirhandle == INVALID_HANDLE_VALUE) {
			err = GetLastError();
			// if the directory was empty we succeeded even though FindFirstFile failed
			return (err == ERROR_FILE_NOT_FOUND);
		}

		do {
			std::string fname = transcode_utf16_to_utf8(findinfo.cFileName, wcslen(findinfo.cFileName));
			if (fname != "." && fname != "..") {
				FileInfo::FileType ty = file_type_for_attributes(findinfo.dwFileAttributes);
				output.push_back(MakeFileInfo(JoinPath(dirpath, fname), ty));
			}

			if (! FindNextFileW(dirhandle, &findinfo)) {
				err = GetLastError();
			} else
				err = ERROR_SUCCESS;
		} while (err == ERROR_SUCCESS);
		FindClose(dirhandle);

		if (err != ERROR_NO_MORE_FILES) {
			output.resize(output_head_size);
			return false;
		}

		std::sort(output.begin() + output_head_size, output.end());
		return true;
	}

	static bool make_directory_raw(std::wstring path)
	{
		// allow multiple tries (to build parent directories)
		while (true) {
			if (!CreateDirectoryW(path.c_str(), 0)) {
				DWORD err = GetLastError();
				if (err == ERROR_ALREADY_EXISTS) {
					DWORD attrs = GetFileAttributesW(path.c_str());
					return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
				} else if (err == ERROR_PATH_NOT_FOUND) {
					if (!PathRemoveFileSpecW(&path[0]))
						return false;
					path.resize(wcslen(&path[0]));
					if (!path.empty() || !make_directory_raw(path)) {
						return false;
					}
				} else {
					fprintf(stderr, "error while attempting to create directory '%s'\n", transcode_utf16_to_utf8(path).c_str());
					return false;
				}
			}
		}
	}

	bool FileSourceFS::MakeDirectory(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		return make_directory_raw(wfullpath);
	}
}

#endif /* _WIN32 */
