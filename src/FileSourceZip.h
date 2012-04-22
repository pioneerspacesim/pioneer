#ifndef _FILESOURCEZIP_H
#define _FILESOURCEZIP_H

#include "FileSystem.h"

namespace FileSystem {

class FileSourceZip : public FileSource {
public:
	FileSourceZip(const std::string &zipFile);
	virtual ~FileSourceZip();

	virtual FileInfo Lookup(const std::string &path);
	virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
	virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);
};

}

#endif
