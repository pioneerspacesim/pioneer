#include "FileSourceZip.h"
#include "miniz.h"

namespace FileSystem {

FileSourceZip::FileSourceZip(const std::string &zipFile) : FileSource(zipFile)
{
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
