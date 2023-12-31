// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSystem.h"
#include "StringRange.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>

namespace FileSystem {

	static FileSourceFS dataFilesApp(GetDataDir(), true);
	static FileSourceFS dataFilesUser(JoinPath(GetUserDir(), "data"));
	FileSourceUnion gameDataFiles;
	FileSourceFS userFiles(GetUserDir());

	// note: some functions (GetUserDir(), GetDataDir()) are in FileSystem{Posix,Win32}.cpp
	std::string SanitiseFileName(const std::string &a)
	{
		const char *disabled_chars = "\\/'\":?<>|&*";
		std::ostringstream ss;
		if (a.empty() || (a[0] == '.')) {
			ss << "x";
		}

		for (const char *c = a.c_str(), *end = c + a.size(); c != end; ++c) {
			if (strchr(disabled_chars, *c) || (*c < ' ')) {
				ss << "_";
				ss.setf(std::ios::hex, std::ios::basefield);
				ss.fill('0');
				ss.width(2);
				ss << int(*c);
			} else if (*c == ' ') {
				ss << '_';
			} else {
				ss << *c;
			}
		}

		return ss.str();
	}

	std::string JoinPath(const std::string &a, const std::string &b)
	{
		if (!b.empty()) {
			if (b[0] == '/' || a.empty())
				return b;
			else
				return a + "/" + b;
		} else
			return a;
	}

	static void normalise_path(std::string &result, const StringRange &path)
	{
		StringRange part(path.begin, path.end);
		if (!path.Empty() && (path[0] == '/')) {
			result += '/';
			++part.begin;
		}
		const size_t initial_result_length = result.size();
		while (true) {
			part.end = part.FindChar('/'); // returns part.end if the char is not found
			if (part.Empty() || (part == ".")) {
				// skip this part
			} else if (part == "..") {
				// pop the last component
				if (result.size() <= initial_result_length)
					throw std::invalid_argument(path.ToString());
				size_t pos = result.rfind('/');
				if (pos == std::string::npos) {
					pos = 0;
				}
				assert(pos >= initial_result_length);
				result.erase(pos);
			} else {
				// push the new component
				if (result.size() > initial_result_length)
					result += '/';
				result.append(part.begin, part.Size());
			}
			if (part.end == path.end) {
				break;
			}
			assert(*part.end == '/');
			part.begin = part.end + 1;
			part.end = path.end;
		}
	}

	std::string NormalisePath(const std::string &path)
	{
		std::string result;
		result.reserve(path.size());
		normalise_path(result, StringRange(path.c_str(), path.size()));
		return result;
	}

	std::string JoinPathBelow(const std::string &base, const std::string &path)
	{
		if (base.empty())
			return path;
		if (!path.empty()) {
			if ((path[0] == '/') && (base != "/"))
				throw std::invalid_argument(path);
			else {
				std::string result(base);
				result.reserve(result.size() + 1 + path.size());
				if (result[result.size() - 1] != '/')
					result += '/';
				StringRange rhs(path.c_str(), path.size());
				if (path[0] == '/') {
					assert(base == "/");
					++rhs.begin;
				}
				normalise_path(result, rhs);
				return result;
			}
		} else
			return base;
	}

	std::string GetRelativePath(const std::string &base, const std::string &path)
	{
		// catch all common errors early
		if (base.empty() || path.empty() || path.size() < base.size())
			return path;

		// can't strip a non-root prefix from a root path and vice-versa
		if ((path[0] == '/' || base[0] == '/') && (path[0] != base[0]))
			return path;

		// check if <path> exactly starts with <base>
		if (path.compare(0, base.size(), base) == 0) {
			if (path.size() == base.size())
				return ""; // strip the entire base and return an empty path
			else if (path[base.size()] == '/')
				return path.substr(base.size() + 1); // strip the base and the trailing separator
		}

		// if <path> isn't relative to <base>, return the original <path>
		return path;
	}

	bool CopyDir(FileSource &sourceFS, std::string sourceDir, FileSourceFS &targetFS, std::string targetDir, FileSystem::CopyMode copymode)
	{
		// NOTE: copymode var is not used, because only mode ONLY_MISSING_IN_TARGET is implemented
		if (!sourceFS.Lookup(sourceDir).IsDir() || !targetFS.Lookup(targetDir).IsDir())
			return false;

		// collect files, that are already in the target
		// NOTE: modification time (in map value) probably will be needed in another mode
		std::map<std::string, Time::DateTime> targetFiles;
		if (copymode != CopyMode::OVERWRITE) {
			// don't bother collecting target file data if we're overwriting
			for (FileSystem::FileEnumerator files(targetFS, targetDir, FileSystem::FileEnumerator::Recurse | FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next())
				targetFiles[files.Current().GetPath()] = files.Current().GetModificationTime();
		}

		for (FileSystem::FileEnumerator files(sourceFS, sourceDir, FileSystem::FileEnumerator::Recurse | FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			const std::string targetPath = FileSystem::JoinPathBelow(targetDir, FileSystem::GetRelativePath(sourceDir, info.GetPath()));
			const auto &oldFile = targetFiles.find(targetPath);
			if (oldFile == targetFiles.end()) { // there is no such file (or dir) in the target
				if (info.IsFile()) {
					//copy file
					RefCountedPtr<FileData> fileData = info.Read();
					FILE *outfile = targetFS.OpenWriteStream(targetPath);
					fwrite(fileData->GetData(), 1, fileData->GetSize(), outfile);
					fclose(outfile);
					// Output("copy %s to %s\n", info.GetAbsolutePath(), FileSystem::JoinPath(targetFS.GetRoot(), targetPath));
				} else if (info.IsDir()) {
					//create the subdir
					targetFS.MakeDirectory(targetPath);
					// Output("create dir %s\n", FileSystem::JoinPath(targetFS.GetRoot(), targetPath));
				}
			} else
				//this file is no longer needed when searching
				targetFiles.erase(oldFile);
		}

		return true;
	}

	void Init()
	{
		gameDataFiles.AppendSource(&dataFilesUser);
		gameDataFiles.AppendSource(&dataFilesApp);
	}

	void Uninit()
	{
	}

	FileInfo::FileInfo(FileSource *source, const std::string &path, FileType type, Time::DateTime modTime) :
		m_source(source),
		m_path(path),
		m_modTime(modTime),
		m_dirLen(0),
		m_type(type)
	{
		if (!m_path.empty() && m_path[m_path.size() - 1] == '/') {
			// remove trailing slash
			m_path.pop_back();
		}

		std::size_t slashpos = m_path.rfind('/');
		if (slashpos != std::string::npos) {
			m_dirLen = slashpos + 1;
		} else {
			m_dirLen = 0;
		}
	}

	FileInfo FileSource::MakeFileInfo(const std::string &path, FileInfo::FileType fileType, Time::DateTime modTime)
	{
		return FileInfo(this, path, fileType, modTime);
	}

	FileInfo FileSource::MakeFileInfo(const std::string &path, FileInfo::FileType fileType)
	{
		return MakeFileInfo(path, fileType, Time::DateTime());
	}

	FileSourceUnion::FileSourceUnion() :
		FileSource(":union:") {}
	FileSourceUnion::~FileSourceUnion() {}

	void FileSourceUnion::PrependSource(FileSource *fs)
	{
		assert(fs);
		RemoveSource(fs);
		m_sources.insert(m_sources.begin(), fs);
	}

	void FileSourceUnion::AppendSource(FileSource *fs)
	{
		assert(fs);
		RemoveSource(fs);
		m_sources.push_back(fs);
	}

	void FileSourceUnion::RemoveSource(FileSource *fs)
	{
		std::vector<FileSource *>::iterator nend = std::remove(m_sources.begin(), m_sources.end(), fs);
		m_sources.erase(nend, m_sources.end());
	}

	FileInfo FileSourceUnion::Lookup(const std::string &path)
	{
		for (std::vector<FileSource *>::const_iterator
				 it = m_sources.begin();
			 it != m_sources.end(); ++it) {
			FileInfo info = (*it)->Lookup(path);
			if (info.Exists()) {
				return info;
			}
		}
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);
	}

	std::vector<FileInfo> FileSourceUnion::LookupAll(const std::string &path)
	{
		std::vector<FileInfo> outFiles;

		for (FileSource *fs : m_sources) {
			FileInfo info = fs->Lookup(path);
			if (info.Exists()) outFiles.push_back(info);
		}

		return outFiles;
	}

	RefCountedPtr<FileData> FileSourceUnion::ReadFile(const std::string &path)
	{
		for (std::vector<FileSource *>::const_iterator
				 it = m_sources.begin();
			 it != m_sources.end(); ++it) {
			RefCountedPtr<FileData> data = (*it)->ReadFile(path);
			if (data) {
				return data;
			}
		}
		return RefCountedPtr<FileData>();
	}

	// Merge two sets of FileInfo's, by path.
	// Input vectors must be sorted. Output will be sorted.
	// Where a path is present in both inputs, directories are selected
	// in preference to non-directories; otherwise, the FileInfo from the
	// first vector is selected in preference to the second vector.
	static void file_union_merge(
		std::vector<FileInfo>::const_iterator a, std::vector<FileInfo>::const_iterator aend,
		std::vector<FileInfo>::const_iterator b, std::vector<FileInfo>::const_iterator bend,
		std::vector<FileInfo> &output)
	{
		while ((a != aend) && (b != bend)) {
			int order = a->GetPath().compare(b->GetPath());
			int which = order;
			if (which == 0) {
				if (b->IsDir() && !a->IsDir()) {
					which = 1;
				} else {
					which = -1;
				}
			}
			if (which < 0) {
				output.push_back(*a++);
				if (order == 0) ++b;
			} else {
				output.push_back(*b++);
				if (order == 0) ++a;
			}
		}

		if (a != aend) {
			std::copy(a, aend, std::back_inserter(output));
		}
		if (b != bend) {
			std::copy(b, bend, std::back_inserter(output));
		}
	}

	bool FileSourceUnion::ReadDirectory(const std::string &path, std::vector<FileInfo> &output)
	{
		if (m_sources.empty()) {
			return false;
		}
		if (m_sources.size() == 1) {
			return m_sources.front()->ReadDirectory(path, output);
		}

		bool founddir = false;

		std::vector<FileInfo> merged;
		for (std::vector<FileSource *>::const_iterator
				 it = m_sources.begin();
			 it != m_sources.end(); ++it) {
			std::vector<FileInfo> nextfiles;
			if ((*it)->ReadDirectory(path, nextfiles)) {
				founddir = true;

				std::vector<FileInfo> prevfiles;
				prevfiles.swap(merged);
				// merge order is important
				// file_union_merge selects from its first input preferentially
				file_union_merge(
					prevfiles.begin(), prevfiles.end(),
					nextfiles.begin(), nextfiles.end(),
					merged);
			}
		}

		output.reserve(output.size() + merged.size());
		std::copy(merged.begin(), merged.end(), std::back_inserter(output));

		return founddir;
	}

	FileEnumerator::FileEnumerator(FileSource &fs, int flags) :
		m_source(&fs),
		m_flags(flags) {}

	FileEnumerator::FileEnumerator(FileSource &fs, const std::string &path, int flags) :
		m_source(&fs),
		m_flags(flags)
	{
		AddSearchRoot(path);
	}

	FileEnumerator::~FileEnumerator() {}

	void FileEnumerator::AddSearchRoot(const std::string &path)
	{
		const FileInfo fi = m_source->Lookup(path);
		if (fi.IsDir()) {
			QueueDirectoryContents(fi);
			ExpandDirQueue();
		}
	}

	void FileEnumerator::Next()
	{
		m_queue.pop_front();
		ExpandDirQueue();
	}

	void FileEnumerator::ExpandDirQueue()
	{
		while (m_queue.empty() && !m_dirQueue.empty()) {
			const FileInfo &nextDir = m_dirQueue.front();
			assert(nextDir.IsDir());
			QueueDirectoryContents(nextDir);
			m_dirQueue.pop_front();
		}
	}

	void FileEnumerator::QueueDirectoryContents(const FileInfo &info)
	{
		assert(info.IsDir());

		std::vector<FileInfo> entries;
		m_source->ReadDirectory(info.GetPath(), entries);
		for (std::vector<FileInfo>::const_iterator
				 it = entries.begin();
			 it != entries.end(); ++it) {

			switch (it->GetType()) {
			case FileInfo::FT_DIR:
				if (m_flags & IncludeDirs) {
					m_queue.push_back(*it);
				}
				if (m_flags & Recurse) {
					m_dirQueue.push_back(*it);
				}
				break;
			case FileInfo::FT_FILE:
				if (!(m_flags & ExcludeFiles)) {
					m_queue.push_back(*it);
				}
				break;
			case FileInfo::FT_SPECIAL:
				if (m_flags & IncludeSpecials) {
					m_queue.push_back(*it);
				}
				break;
			default: assert(0); break;
			}
		}
	}

} // namespace FileSystem
