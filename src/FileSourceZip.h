// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FILESOURCEZIP_H
#define _FILESOURCEZIP_H

#include "FileSystem.h"
#include <SDL_stdinc.h>
#include <map>
#include <string>

namespace FileSystem {

	class FileSourceZip : public FileSource {
	public:
		// for now this needs to be FileSourceFS rather than just FileSource,
		// because we need a FILE* stream access to the .zip file
		FileSourceZip(FileSourceFS &fs, const std::string &zipPath);
		virtual ~FileSourceZip();

		virtual FileInfo Lookup(const std::string &path);
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

	private:
		void *m_archive;

		struct FileStat {
			FileStat(Uint32 _index, Uint64 _size, const FileInfo &_info) :
				index(_index),
				size(_size),
				info(_info) {}
			const Uint32 index;
			const Uint64 size;
			const FileInfo info;
		};

		struct Directory {
			std::map<std::string, Directory> subdirs;
			std::map<std::string, FileStat> files;
		};

		Directory m_root;

		bool FindDirectoryAndFile(const std::string &path, const Directory *&dir, std::string &filename);
		void AddFile(const std::string &path, const FileStat &fileStat);
	};

} // namespace FileSystem

#endif
