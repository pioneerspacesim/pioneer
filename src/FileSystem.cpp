#include "libs.h"
#include "FileSystem.h"
#include "StringRange.h"
#include <cassert>
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace FileSystem {

	static FileSourceFS dataFilesApp(GetDataDir());
	static FileSourceFS dataFilesUser(GetUserDir("data"));
	FileSourceUnion gameDataFiles;
	FileSourceFS rawFileSystem("/");

	// note: some functions (GetUserDir(), GetDataDir()) are in FileSystem{Posix,Win32}.cpp

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
		StringRange part(path.begin, path.begin);
		const char *c = path.begin;
		if ((c != path.end) && (*c == '/')) {
			result += '/';
			++c;
			++part.begin;
		}
		const size_t initial_result_length = result.size();
		while (true) {
			if ((*c == '/') || (c == path.end)) {
				part.end = c;
				if (part.Empty() || (part == ".")) {
					// skip this part
				} else if (part == "..") {
					// pop the last component
					if (result.size() <= initial_result_length)
						throw std::invalid_argument(path.ToString());
					size_t pos = result.rfind('/', result.size()-2);
					++pos;
					assert(pos >= initial_result_length);
					result.erase(pos);
				} else {
					// push the new component
					if (part.end != path.end) {
						assert(*part.end == '/');
						++part.end;
					}
					result.append(part.begin, part.Size());
				}
				part.begin = c+1;
			}
			if (c == path.end) { break; }
			++c;
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
				if (result[result.size()-1] != '/')
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

	void Init()
	{
		gameDataFiles.AppendSource(&dataFilesUser);
		gameDataFiles.AppendSource(&dataFilesApp);
	}

	void Uninit()
	{
	}

	FileInfo::FileInfo(FileSource *source, const std::string &path, FileType type):
		m_source(source),
		m_path(path),
		m_dirLen(0),
		m_type(type)
	{
		std::size_t slashpos = m_path.rfind('/');
		if (slashpos != std::string::npos) {
			m_dirLen = slashpos + 1;
		} else {
			m_dirLen = 0;
		}
	}

	FileInfo FileSource::MakeFileInfo(const std::string &path, FileInfo::FileType fileType)
	{
		return FileInfo(this, path, fileType);
	}

	FileSourceUnion::FileSourceUnion(): FileSource(":union:") {}
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
		std::vector<FileSource*>::iterator nend = std::remove(m_sources.begin(), m_sources.end(), fs);
		m_sources.erase(nend, m_sources.end());
	}

	FileInfo FileSourceUnion::Lookup(const std::string &path)
	{
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			FileInfo info = (*it)->Lookup(path);
			if (info.Exists()) { return info; }
		}
		return MakeFileInfo(path, FileInfo::FT_NON_EXISTENT);
	}

	RefCountedPtr<FileData> FileSourceUnion::ReadFile(const std::string &path)
	{
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			RefCountedPtr<FileData> data = (*it)->ReadFile(path);
			if (data) { return data; }
		}
		return RefCountedPtr<FileData>();
	}

	static void file_union_merge(
			std::vector<FileInfo>::const_iterator a, std::vector<FileInfo>::const_iterator aend,
			std::vector<FileInfo>::const_iterator b, std::vector<FileInfo>::const_iterator bend,
			std::vector<FileInfo> &output)
	{
		while (true) {
			if (a == aend) {
				std::copy(b, bend, std::back_inserter(output));
				return;
			}
			if (b == bend) {
				std::copy(a, aend, std::back_inserter(output));
				return;
			}

			int c = a->GetPath().compare(b->GetPath());
			int which = c;
			if (which == 0) {
				if (b->IsDir() && !a->IsDir()) { which = 1; }
				else { which = -1; }
			}
			if (which < 0) {
				output.push_back(*a++);
				if (c == 0) ++b;
			} else {
				output.push_back(*b++);
				if (c == 0) ++a;
			}
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
		size_t headsize = output.size();

		std::vector<FileInfo> merged;
		for (std::vector<FileSource*>::const_iterator
			it = m_sources.begin(); it != m_sources.end(); ++it)
		{
			size_t prevsize = output.size();
			assert(prevsize >= headsize);
			std::vector<FileInfo> temp1;
			if ((*it)->ReadDirectory(path, temp1))
				founddir = true;

			std::vector<FileInfo> temp2;
			temp2.swap(merged);
			file_union_merge(
				temp1.begin(), temp1.end(),
				temp2.begin(), temp2.end(),
				merged);
		}

		output.reserve(output.size() + merged.size());
		std::copy(merged.begin(), merged.end(), std::back_inserter(output));

		return founddir;
	}

	FileEnumerator::FileEnumerator(FileSource &fs, int flags):
		m_source(&fs), m_flags(flags)
	{
		Init("/");
	}

	FileEnumerator::FileEnumerator(FileSource &fs, const std::string &path, int flags):
		m_source(&fs), m_flags(flags)
	{
		Init(path);
	}

	FileEnumerator::~FileEnumerator() {}

	void FileEnumerator::Init(const std::string &path)
	{
		FileInfo fi = m_source->Lookup(path);
		if (fi.IsDir()) {
			m_queue.push_back(fi);
			Next(m_flags | Recurse);
		}
	}

	void FileEnumerator::Next(int flags)
	{
		if (flags & RecurseFlag) {
			FileInfo head = m_queue.front();
			m_queue.pop_front();

			if (head.IsDir()) {
				std::vector<FileInfo> entries;
				m_source->ReadDirectory(head.GetPath(), entries);
				for (std::vector<FileInfo>::const_iterator
					it = entries.begin(); it != entries.end(); ++it) {

					if ((flags & IncludeDirectories) && it->IsDir())
						m_queue.push_back(*it);
					if (!(flags & ExcludeFiles) && it->IsFile())
						m_queue.push_back(*it);
				}
			}
		} else {
			m_queue.pop_front();
		}
	}

} // namespace FileSystem
