#include "libs.h"
#include "FileSystem.h"
#include <cassert>
#include <algorithm>

namespace FileSystem {

	// note: GetUserDir() and GetDataDir() is in FileSystem{Posix,Win32}.cpp

	FileInfo::FileInfo(FileSource *source, const std::string &path, FileType type):
		m_source(source),
		m_path(path),
		m_dirLen(0),
		m_type(type) {
		std::size_t slashpos = m_path.rfind('/');
		if (slashpos != std::string::npos) {
			m_dirLen = slashpos + 1;
		} else {
			m_dirLen = 0;
		}
	}

	FileInfo FileSource::MakeFileInfo(const std::string &path, FileInfo::FileType fileType) {
		return FileInfo(this, path, fileType);
	}

	FileSourceUnion::FileSourceUnion(): FileSource(":union:") {}
	FileSourceUnion::~FileSourceUnion() {}

	void FileSourceUnion::PrependSource(FileSource *fs) {
		assert(fs);
		RemoveSource(fs);
		m_sources.insert(m_sources.begin(), fs);
	}

	void FileSourceUnion::AppendSource(FileSource *fs) {
		assert(fs);
		RemoveSource(fs);
		m_sources.push_back(fs);
	}

	void FileSourceUnion::RemoveSource(FileSource *fs) {
		std::vector<FileSource*>::iterator nend = std::remove(m_sources.begin(), m_sources.end(), fs);
		m_sources.erase(nend, m_sources.end());
	}

	FileInfo FileSourceUnion::Lookup(const std::string &path) {
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			FileInfo info = (*it)->Lookup(path);
			if (info.Exists()) { return info; }
		}
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);
	}

	RefCountedPtr<FileData> FileSourceUnion::ReadFile(const std::string &path) {
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			RefCountedPtr<FileData> data = (*it)->ReadFile(path);
			if (data) { return data; }
		}
		return RefCountedPtr<FileData>();
	}

	struct FileInfoLessPathType
		: public std::binary_function<const FileInfo&, const FileInfo&, bool>
	{
		// less-than predicate
		// this orders by path, and then by type, with directories appearing before files
		bool operator()(const FileInfo &a, const FileInfo &b) const
		{
			int c = a.GetPath().compare(b.GetPath());
			if (c != 0) { return (c < 0); }
			return (a.IsDir() && !b.IsDir());
		}
	};

	struct FileInfoEqualPaths
		: public std::binary_function<const FileInfo&, const FileInfo&, bool>
	{
		// equality predicate
		// compares only the path
		bool operator()(const FileInfo &a, const FileInfo &b) const
		{ return (a.GetPath() == b.GetPath()); }
	};

	void FileSourceUnion::ReadDirectory(const std::string &path, std::vector<FileInfo> &output) {
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			(*it)->ReadDirectory(path, output);
		}
		// merge
		std::sort(output.begin(), output.end(), FileInfoLessPathType());
		// remove duplicates
		std::vector<FileInfo>::iterator nend = std::unique(output.begin(), output.end(), FileInfoEqualPaths());
		output.erase(nend, output.end());
	}

}
