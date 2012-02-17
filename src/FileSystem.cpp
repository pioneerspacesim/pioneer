#include "libs.h"
#include "FileSystem.h"
#include <cassert>

namespace FileSystem {

	// note: GetUserDir() and GetDataDir() is in FileSystem{Posix,Win32}.cpp

	FileInfo::FileInfo(const FileSource *source, const std::string &path, FileType type):
		m_source(source),
		m_path(path),
		m_dirLen(0),
		m_fileType(type) {
		std::size_t slashpos = m_path.rfind('/');
		if (slashpos != std::string::npos) {
			m_dirLen = slashpos + 1;
		} else {
			m_dirLen = 0;
		}
	}

	FileSource::FileSource(const std::string &sourcePath):
		m_sourcePath(sourcePath) {}

	FileSource::~FileSource() {}

	FileInfo FileSource::MakeFileInfo(const std::string &path, FileInfo::FileType fileType) {
		return FileInfo(this, path, fileType);
	}

}
