#include "FileSourceZip.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

extern "C" {
#include "miniz/miniz.h"
}

#undef FT_FILE // TODO FileInfo::FT_FILE is conflicting with a FreeType def; undefine it for now

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
			if (!mz_zip_reader_is_file_encrypted(zip, i)) {
				std::string fname = zipStat.m_filename;
				if ((fname.size() > 1) && (fname[fname.size()-1] == '/')) {
					fname.resize(fname.size() - 1);
				}
				AddFile(zipStat.m_filename, FileStat(i, zipStat.m_uncomp_size,
					MakeFileInfo(fname, is_dir ? FileInfo::FT_DIR : FileInfo::FT_FILE)));
			}
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

static void SplitPath(const std::string &path, std::vector<std::string> &output)
{
	static const std::string delim("/");

	size_t start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = path.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = path.find_first_of(delim, start);

		// extract the fragment and remember it
		output.push_back(path.substr(start, (end == std::string::npos) ? std::string::npos : end - start));
	}
}

bool FileSourceZip::FindDirectoryAndFile(const std::string &path, const Directory* &dir, std::string &filename)
{
	std::vector<std::string> fragments;
	SplitPath(NormalisePath(path), fragments);

	assert(fragments.size() > 0);

	dir = &m_root;

	if (fragments.size() > 1) {
		for (unsigned int i = 0; i < fragments.size()-1; i++) {
			std::map<std::string,Directory>::const_iterator it = dir->subdirs.find(fragments[i]);
			if (it == dir->subdirs.end())
				return false;
			dir = &((*it).second);
		}
	}

	filename = fragments.back();
	return true;
}

FileInfo FileSourceZip::Lookup(const std::string &path)
{
	const Directory *dir;
	std::string filename;
	if (!FindDirectoryAndFile(path, dir, filename))
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);

	std::map<std::string,FileStat>::const_iterator i = dir->files.find(filename);
	if (i == dir->files.end())
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);

	return (*i).second.info;
}

RefCountedPtr<FileData> FileSourceZip::ReadFile(const std::string &path)
{
	if (!m_archive) return RefCountedPtr<FileData>();
	mz_zip_archive *zip = reinterpret_cast<mz_zip_archive*>(m_archive);

	const Directory *dir;
	std::string filename;
	if (!FindDirectoryAndFile(path, dir, filename))
		return RefCountedPtr<FileData>();

	std::map<std::string,FileStat>::const_iterator i = dir->files.find(filename);
	if (i == dir->files.end())
		return RefCountedPtr<FileData>();

	const FileStat &st = (*i).second;

	char *data = reinterpret_cast<char*>(std::malloc(st.size));
	if (!mz_zip_reader_extract_to_mem(zip, st.index, data, st.size, 0)) {
		printf("FileSourceZip::ReadFile: couldn't extract '%s'\n", path.c_str());
		return RefCountedPtr<FileData>();
	}

	return RefCountedPtr<FileData>(new FileDataMalloc(st.info, st.size, data));
}

bool FileSourceZip::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
{
	const Directory *dir;
	std::string filename;
	if (!FindDirectoryAndFile(path, dir, filename))
		return false;

	{
		std::map<std::string,Directory>::const_iterator i = dir->subdirs.find(filename);
		if (i == dir->subdirs.end())
			return false;
		dir = &((*i).second);
	}

	for (std::map<std::string,FileStat>::const_iterator i = dir->files.begin(); i != dir->files.end(); ++i)
		output.push_back((*i).second.info);

	return true;
}

void FileSourceZip::AddFile(const std::string &path, const FileStat &fileStat)
{
	std::vector<std::string> fragments;
	SplitPath(path, fragments);

	assert(fragments.size() > 0);

	Directory *dir = &m_root;

	if (fragments.size() > 1) {
		std::string fullPath;

		for (unsigned int i = 0; i < fragments.size()-1; i++) {
			fullPath += ((i > 0) ? "/" : "") + fragments[i];

			std::map<std::string,FileStat>::const_iterator it = dir->files.find(fragments[i]);
			if (it == dir->files.end())
				dir->files.insert(std::make_pair(fragments[i], FileStat(Uint32(-1), 0, MakeFileInfo(fullPath, FileInfo::FT_DIR))));
			dir = &(dir->subdirs[fragments[i]]);
		}
	}

	const std::string &filename = fragments.back();

	if (fileStat.info.IsDir())
		dir->subdirs.insert(std::make_pair(filename, Directory()));

	dir->files.insert(std::make_pair(filename, fileStat));
}

}
