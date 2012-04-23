#include "FileSourceZip.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

extern "C" {
#include "miniz.h"
}

namespace FileSystem {

FileSourceZip::FileSourceZip(const std::string &zipPath) : FileSource(zipPath), m_archive(0)
{
	mz_zip_archive *zip = reinterpret_cast<mz_zip_archive*>(std::calloc(1, sizeof(mz_zip_archive)));
	if (!mz_zip_reader_init_file(zip, zipPath.c_str(), 0)) {
		printf("FileSourceZip: unable to open '%s'\n", zipPath.c_str());
		std::free(zip);
		return;
	}

	mz_zip_archive_file_stat zipStat;

	Uint32 numFiles = mz_zip_reader_get_num_files(zip);
	for (Uint32 i = 0; i < numFiles; i++) {
		if (mz_zip_reader_file_stat(zip, i, &zipStat)) {
			bool is_dir = mz_zip_reader_is_file_a_directory(zip, i);
			if (!mz_zip_reader_is_file_encrypted(zip, i))
				m_index.insert(std::make_pair(zipStat.m_filename, FileStat(i, zipStat.m_uncomp_size, MakeFileInfo(zipStat.m_filename, is_dir ? FileInfo::FT_DIR : FileInfo::FT_FILE))));
		}
	}

	m_archive = reinterpret_cast<void*>(zip);
}

FileSourceZip::~FileSourceZip()
{
	if (!m_archive) return;
	mz_zip_archive *zip = reinterpret_cast<mz_zip_archive*>(m_archive);
	mz_zip_reader_end(zip);
}

FileInfo FileSourceZip::Lookup(const std::string &path)
{
	FileMap::iterator i = m_index.find(NormalisePath(path));
	if (i == m_index.end()) return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);
	return (*i).second.info;
}

RefCountedPtr<FileData> FileSourceZip::ReadFile(const std::string &path)
{
	if (!m_archive) return RefCountedPtr<FileData>();
	mz_zip_archive *zip = reinterpret_cast<mz_zip_archive*>(m_archive);

	FileMap::iterator i = m_index.find(NormalisePath(path));
	if (i == m_index.end()) return RefCountedPtr<FileData>();

	FileStat st = (*i).second;

	char *data = reinterpret_cast<char*>(std::malloc(st.size));
	if (!mz_zip_reader_extract_to_mem(zip, st.index, data, st.size, 0)) {
		printf("FileSourceZip::ReadFile: couldn't extract '%s'\n", path.c_str());
		return RefCountedPtr<FileData>();
	}

	return RefCountedPtr<FileData>(new FileDataMalloc(st.info, st.size, data));
}

bool FileSourceZip::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
{
	const std::string base(NormalisePath(path)+"/");
	if (m_index.find(base) == m_index.end()) return false;

	for (FileMap::iterator i = m_index.begin(); i != m_index.end(); ++i)
		if ((*i).first.size() > base.size() && (*i).first.substr(0, base.size()) == base)
			output.push_back((*i).second.info);

	std::sort(output.begin(), output.end());

	return true;
}

}
