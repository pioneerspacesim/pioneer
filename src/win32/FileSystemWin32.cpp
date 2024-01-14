// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Win32Setup.h"

#include "FileSystem.h"
#include "TextUtils.h"
#include "libs.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <cerrno>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// GetPiUserDir() needs these
#include <shlobj.h>
#include <shlwapi.h>

namespace FileSystem {
	const wchar_t FORBIDDEN_CHARACTERS[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28,
		29, 30, 31, L'\\', L'/', L'<', L'>', L':',
		L'"', L'|', L'?', L'*'
	};

	static std::string absolute_path(const std::string &path)
	{
		std::wstring wpath = transcode_utf8_to_utf16(path);
		wchar_t buf[MAX_PATH + 1];
		DWORD len = GetFullPathNameW(wpath.c_str(), MAX_PATH, buf, 0);
		buf[len] = L'\0';
		return transcode_utf16_to_utf8(buf, len);
	}

	static std::string FindUserDir()
	{
		wchar_t appdata_path[MAX_PATH];
		if (SHGetFolderPathW(0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, appdata_path) != S_OK) {
			Output("Couldn't get user documents folder path\n");
			exit(-1);
		}

		std::wstring path(appdata_path);
		path += L"/Pioneer";

		if (!PathFileExistsW(path.c_str())) {
			if (SHCreateDirectoryExW(0, path.c_str(), 0) != ERROR_SUCCESS) {
				std::string utf8path = transcode_utf16_to_utf8(path);
				Output("Couldn't create user game folder '%s'", utf8path.c_str());
				exit(-1);
			}
		}

		return transcode_utf16_to_utf8(path);
	}

	static std::string FindDataDir()
	{
		static const DWORD BUFSIZE = MAX_PATH + 1;
		wchar_t buf[BUFSIZE];
		memset(buf, L'\0', sizeof(wchar_t) * BUFSIZE); // clear the buffer
		DWORD dwRet = GetCurrentDirectoryW(BUFSIZE, buf);
		if (dwRet == 0) {
			printf("GetCurrentDirectory failed (%d)\n", GetLastError());
			abort();
		}
		if (dwRet > BUFSIZE) {
			printf("Buffer too small; need %d characters\n", dwRet);
			// I gave you MAX_PATH, what more do you want!?
			abort();
		}
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// I gave you MAX_PATH, what more do you want!?
			abort();
		}
		PathAppendW(buf, L"data");
		return transcode_utf16_to_utf8(buf, wcslen(buf));
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

	bool IsValidFilename(const std::string &filename)
	{
		const std::wstring wfilename = transcode_utf8_to_utf16(filename);

		// Filename shouldn't end with space or dot on Windows
		if (wfilename[wfilename.size() - 1] == L' ' || wfilename[wfilename.size() - 1] == L'.')
			return false;

		for (wchar_t c : FORBIDDEN_CHARACTERS) {
			if (wfilename.find(c) != std::string::npos)
				return false;
		}
		return true;
	}

	FileSourceFS::FileSourceFS(const std::string &root, bool trusted) :
		FileSource((root == "/") ? "" : absolute_path(root), trusted) {}

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

	static Time::DateTime datetime_for_filetime(FILETIME filetime)
	{
		Time::DateTime dt;

		SYSTEMTIME systime, localtime;
		FileTimeToSystemTime(&filetime, &systime);
		SystemTimeToTzSpecificLocalTime(0, &systime, &localtime);

		dt = Time::DateTime(
			localtime.wYear, localtime.wMonth, localtime.wDay,
			localtime.wHour, localtime.wMinute, localtime.wSecond);
		dt += Time::TimeDelta(localtime.wMilliseconds, Time::Millisecond);
		return dt;
	}

	static Time::DateTime file_modtime_for_handle(HANDLE hfile)
	{
		assert(hfile != INVALID_HANDLE_VALUE);

		Time::DateTime modtime;
		FILETIME filetime;
		if (GetFileTime(hfile, 0, 0, &filetime)) {
			modtime = datetime_for_filetime(filetime);
		}
		return modtime;
	}

	FileInfo FileSourceFS::Lookup(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		DWORD attrs = GetFileAttributesW(wfullpath.c_str());
		const FileInfo::FileType ty = file_type_for_attributes(attrs);
		Time::DateTime modtime;
		if (ty == FileInfo::FT_FILE || ty == FileInfo::FT_DIR) {
			HANDLE hfile = CreateFileW(wfullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hfile != INVALID_HANDLE_VALUE) {
				modtime = file_modtime_for_handle(hfile);
				CloseHandle(hfile);
			}
		}
		return MakeFileInfo(path, ty, modtime);
	}

	RefCountedPtr<FileData> FileSourceFS::ReadFile(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		HANDLE filehandle = CreateFileW(wfullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (filehandle == INVALID_HANDLE_VALUE)
			return RefCountedPtr<FileData>(0);
		else {
			const Time::DateTime modtime = file_modtime_for_handle(filehandle);

			LARGE_INTEGER large_size;
			if (!GetFileSizeEx(filehandle, &large_size)) {
				Output("failed to get file size for '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}
			size_t size = size_t(large_size.QuadPart);

			char *data = static_cast<char *>(std::malloc(size));
			if (!data) {
				// XXX handling memory allocation failure gracefully is too hard right now
				Output("failed when allocating buffer for '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}

			if (size > 0x7FFFFFFFull) {
				Output("file '%s' is too large (can't currently cope with files > 2GB)\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}

			DWORD read_size;
			BOOL ret = ::ReadFile(filehandle, static_cast<LPVOID>(data), (DWORD)size, &read_size, 0);
			if (!ret) {
				Output("error while reading file '%s'\n", fullpath.c_str());
				CloseHandle(filehandle);
				abort();
			}
			if (size_t(read_size) != size) {
				Output("file '%s' truncated\n", fullpath.c_str());
				memset(data + read_size, 0xee, size - read_size);
			}

			CloseHandle(filehandle);

			return RefCountedPtr<FileData>(new FileDataMalloc(MakeFileInfo(path, FileInfo::FT_FILE, modtime), size, data));
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
				const FileInfo::FileType ty = file_type_for_attributes(findinfo.dwFileAttributes);
				const Time::DateTime modtime = datetime_for_filetime(findinfo.ftLastWriteTime);
				output.push_back(MakeFileInfo(JoinPath(dirpath, fname), ty, modtime));
			}

			if (!FindNextFileW(dirhandle, &findinfo)) {
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
					Output("error while attempting to create directory '%s'\n", transcode_utf16_to_utf8(path).c_str());
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

	static FILE *open_file_raw(const std::string &fullpath, const wchar_t *mode)
	{
		const std::wstring wfullpath = transcode_utf8_to_utf16(fullpath);
		return _wfopen(wfullpath.c_str(), mode);
	}

	FILE *FileSourceFS::OpenReadStream(const std::string &path)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		return open_file_raw(fullpath, L"rb");
	}

	FILE *FileSourceFS::OpenWriteStream(const std::string &path, int flags)
	{
		const std::string fullpath = JoinPathBelow(GetRoot(), path);
		return open_file_raw(fullpath, (flags & WRITE_TEXT) ? L"w" : L"wb");
	}

	bool FileSourceFS::IsChildOfRoot(const std::string &path)
	{
		if (path.empty())
			return false;
		const std::wstring wpath = transcode_utf8_to_utf16(path);
		if (GetFileAttributesW(wpath.c_str()) == INVALID_FILE_ATTRIBUTES)
			return false;
		const std::unique_ptr<WCHAR[]> fullPath = std::make_unique<WCHAR[]>(MAX_PATH);
		if (!GetFullPathNameW(wpath.c_str(), MAX_PATH, fullPath.get(), NULL))
			return false;
		const std::wstring wroot = transcode_utf8_to_utf16(GetRoot());
		std::wstring pathCompareStr = fullPath.get();
		const std::wstring wrootDrive = wroot.substr(0, wroot.find_first_of(L"/\\"));
		const std::wstring fullPathDrive = pathCompareStr.substr(0, pathCompareStr.find_first_of(L"/\\"));
		if (wrootDrive != fullPathDrive)
			return false;
		size_t elem = 0;
		while ((elem = pathCompareStr.find_last_of(L"/\\")) != std::string::npos) {
			if (pathCompareStr == wroot)
				return true;
			pathCompareStr.resize(elem);
			if (pathCompareStr.size() < wroot.size())
				return false;
		}
		return false;
	}

	bool FileSourceFS::RemoveFile(const std::string &relativePath)
	{
		if (relativePath.empty())
			return false;
		std::wstring combinedPath;
		try {
			combinedPath = transcode_utf8_to_utf16(JoinPathBelow(GetRoot(), relativePath));
		} catch (const std::invalid_argument &) {
			return false;
		}
		const DWORD fileAttributes = GetFileAttributesW(combinedPath.c_str());
		if (file_type_for_attributes(fileAttributes) != FileSystem::FileInfo::FileType::FT_FILE)
			return false;
		if (!IsChildOfRoot(transcode_utf16_to_utf8(combinedPath)))
			return false;
		return DeleteFileW(combinedPath.c_str());
	}
} // namespace FileSystem
