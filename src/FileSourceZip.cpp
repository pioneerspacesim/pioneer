#include "FileSourceZip.h"
#include "miniz.h"
#include <cstdio>

namespace FileSystem {

FileSourceZip::FileSourceZip(const std::string &zipPath) : FileSource(zipPath)
{
	printf("FileSourceZip: %s\n", zipPath.c_str());
}

FileSourceZip::~FileSourceZip()
{
}

FileInfo FileSourceZip::Lookup(const std::string &path)
{
}

RefCountedPtr<FileData> FileSourceZip::ReadFile(const std::string &path)
{
}

bool FileSourceZip::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
{
}

}
