#ifndef _FILESOURCEZIP_H
#define _FILESOURCEZIP_H

#include "FileSystem.h"
#include <SDL_stdinc.h>
#include <map>
#include <string>

namespace FileSystem {

class FileSourceZip : public FileSource {
public:
	FileSourceZip(const std::string &zipPath);
	virtual ~FileSourceZip();

	virtual FileInfo Lookup(const std::string &path);
	virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
	virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

private:
	void *m_archive;

	struct FileStat {
		FileStat(Uint32 _index, Uint64 _size, FileInfo _info) : index(_index), size(_size), info(_info) {}
		const Uint32 index;
		const Uint64 size;
		const FileInfo info;
	};

	typedef std::map<std::string,FileStat> FileMap;
	FileMap m_index;
};

}

#endif
