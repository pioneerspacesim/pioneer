// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "ByteRange.h"
#include "DateTime.h"
#include "RefCounted.h"
#include "StringRange.h"
#include <deque>
#include <memory>
#include <string>
#include <vector>

/*
 * Functionality:
 *   - Overlay multiple file sources (directories and archives)
 *     in a single virtual file system.
 *   - Enumerate entries from that combined virtual file system.
 *   - Read a file from the virtual file system.
 *   - Determine the path of the application's installed files.
 *   - Determine a sensible path for application configuration files.
 */

namespace FileSystem {

	class FileSource;
	class FileData;
	class FileSourceFS;
	class FileSourceUnion;

	void Init();
	void Uninit();

	extern FileSourceUnion gameDataFiles;
	extern FileSourceFS userFiles;

	std::string GetUserDir();
	std::string GetDataDir();
	bool IsValidFilename(const std::string &fileName);

	/// Makes a string safe for use as a file name
	/// warning: this mapping is non-injective, that is,
	/// multiple input names may produce the same output
	std::string SanitiseFileName(const std::string &a);

	/// Create path <a>/<b>, coping with 'a' or 'b' being empty,
	/// 'b' being an absolute path, or 'a' not having a trailing separator
	std::string JoinPath(const std::string &a, const std::string &b);

	/// Create path <base>/<path> ensuring that the result points
	/// to a path at or below <base>
	/// throws an exception (std::invalid_argument) if the path tries to escape
	/// <base> must not be empty
	std::string JoinPathBelow(const std::string &base, const std::string &path);

	/// Given a path in the form <base>/<relpath>, return <relpath>.
	/// If <base> is not the exact base of <path>, return <path> unchanged
	std::string GetRelativePath(const std::string &base, const std::string &path);

	/// Collapse redundant path separators, and '.' and '..' components
	/// NB: this does not interpret symlinks, so the result may refer to
	/// an entirely different file than the input
	/// throws std::invalid_argument if the input path resolves to a 'negative' path
	/// (e.g., "a/../.." resolves to a negative path)
	std::string NormalisePath(const std::string &path);

	enum class CopyMode {
		OVERWRITE,			   // overwrite all files in target with files from source
		ONLY_MISSING_IN_TARGET // only copy files that aren't present in target
	};

	/// Copy the contents of a directory from sourceFS into a directory in targetFS, according to copymode.
	/// Returns false if sourceDir or targetDir are invalid
	bool CopyDir(FileSource &sourceFS, std::string sourceDir, FileSourceFS &targetFS, std::string targetDir, CopyMode copymode = CopyMode::OVERWRITE);

	class FileInfo {
		friend class FileSource;

	public:
		FileInfo() :
			m_source(0),
			m_dirLen(0),
			m_type(FT_NON_EXISTENT) {}

		enum FileType {
			// note: order here affects sort-order of FileInfo
			FT_DIR,
			FT_FILE,
			FT_NON_EXISTENT,
			FT_SPECIAL
		};

		enum FileType GetType() const { return m_type; }
		bool Exists() const { return (m_type != FT_NON_EXISTENT); }
		bool IsDir() const { return (m_type == FT_DIR); }
		bool IsFile() const { return (m_type == FT_FILE); }
		bool IsSpecial() const { return (m_type == FT_SPECIAL); }

		// modification time specified in *local* time (not UTC)
		// (specified in local time because we want it to be easy to display)
		Time::DateTime GetModificationTime() const { return m_modTime; }

		const std::string &GetPath() const { return m_path; }
		std::string GetName() const { return m_path.substr(m_dirLen); }
		std::string GetDir() const { return m_path.substr(0, m_dirLen); }
		std::string GetAbsoluteDir() const;
		std::string GetAbsolutePath() const;

		const FileSource &GetSource() const { return *m_source; }

		RefCountedPtr<FileData> Read() const;

		friend bool operator==(const FileInfo &a, const FileInfo &b)
		{
			return (a.m_source == b.m_source && a.m_type == b.m_type && a.m_path == b.m_path);
		}
		friend bool operator!=(const FileInfo &a, const FileInfo &b)
		{
			return (a.m_source != b.m_source || a.m_type != b.m_type || a.m_path != b.m_path);
		}
		friend bool operator<(const FileInfo &a, const FileInfo &b)
		{
			int c = a.m_path.compare(b.m_path);
			if (c != 0) {
				return (c < 0);
			}
			if (a.m_type != b.m_type) {
				return (a.m_type < b.m_type);
			}
			return (a.m_source < b.m_source);
		}
		friend bool operator<=(const FileInfo &a, const FileInfo &b)
		{
			int c = a.m_path.compare(b.m_path);
			if (c != 0) {
				return (c < 0);
			}
			if (a.m_type != b.m_type) {
				return (a.m_type < b.m_type);
			}
			return (a.m_source <= b.m_source);
		}
		friend bool operator>(const FileInfo &a, const FileInfo &b) { return (b < a); }
		friend bool operator>=(const FileInfo &a, const FileInfo &b) { return (b <= a); }

	private:
		// use FileSource::MakeFileInfo to create your FileInfos
		FileInfo(FileSource *source, const std::string &path, FileType type, Time::DateTime modTime);

		FileSource *m_source;
		std::string m_path;
		Time::DateTime m_modTime;
		int m_dirLen;
		FileType m_type;
	};

	class FileData : public RefCounted {
	public:
		virtual ~FileData() {}

		const FileInfo &GetInfo() const { return m_info; }
		size_t GetSize() const { return m_size; }
		const char *GetData() const
		{
			assert(m_info.IsFile());
			return m_data;
		}
		StringRange AsStringRange() const { return StringRange(m_data, m_size); }
		ByteRange AsByteRange() const { return ByteRange(m_data, m_size); }
		std::string_view AsStringView() const { return std::string_view(m_data, m_size); }

	protected:
		FileData(const FileInfo &info, size_t size, char *data) :
			m_info(info),
			m_data(data),
			m_size(size) {}
		FileData(const FileInfo &info) :
			m_info(info),
			m_data(0),
			m_size(0) {}

		FileInfo m_info;
		char *m_data;
		size_t m_size;
	};

	class FileDataMalloc : public FileData {
	public:
		FileDataMalloc(const FileInfo &info, size_t size) :
			FileData(info, size, static_cast<char *>(std::malloc(size))) {}
		FileDataMalloc(const FileInfo &info, size_t size, char *data) :
			FileData(info, size, data) {}
		virtual ~FileDataMalloc() { std::free(m_data); }
	};

	class FileEnumerator {
	public:
		enum Flags {
			IncludeDirs = 1,
			IncludeSpecials = 2,
			ExcludeFiles = 4,
			Recurse = 8
		};

		// Iterator interface for use with C++11 range-for only
		struct iter {
			FileEnumerator &m_enum;
			bool m_atend = false;

			using iterator_category = std::input_iterator_tag;
			using reference = const FileInfo &;
			using value_type = const FileInfo;

			reference operator*() const { return m_enum.Current(); }
			bool operator!=(const iter &rhs) { return m_enum.Finished() != rhs.m_atend; }

			iter &operator++()
			{
				m_enum.Next();
				return *this;
			}
		};

		explicit FileEnumerator(FileSource &fs, int flags = 0);
		explicit FileEnumerator(FileSource &fs, const std::string &path, int flags = 0);
		~FileEnumerator();

		void AddSearchRoot(const std::string &path);

		bool Finished() const { return m_queue.empty(); }
		void Next();
		const FileInfo &Current() const { return m_queue.front(); }

		iter begin() { return iter{ *this, false }; }
		iter end() { return iter{ *this, true }; }

	private:
		void ExpandDirQueue();
		void QueueDirectoryContents(const FileInfo &info);

		FileSource *m_source;
		std::deque<FileInfo> m_queue;
		std::deque<FileInfo> m_dirQueue;
		int m_flags;
	};

	class FileSource {
	public:
		explicit FileSource(const std::string &root, bool trusted = false) :
			m_root(root),
			m_trusted(trusted) {}
		virtual ~FileSource() {}

		const std::string &GetRoot() const { return m_root; }

		virtual FileInfo Lookup(const std::string &path) = 0;
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path) = 0;
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output) = 0;

		virtual FileEnumerator Enumerate(int enumeratorFlags)
		{
			return FileEnumerator(*this, enumeratorFlags);
		}

		virtual FileEnumerator Enumerate(const std::string &path, int enumeratorFlags)
		{
			return FileEnumerator(*this, path, enumeratorFlags);
		}

		virtual FileEnumerator Recurse(const std::string &path, int enumeratorFlags = 0)
		{
			return FileEnumerator(*this, path, FileEnumerator::Recurse | enumeratorFlags);
		}

		bool IsTrusted() const { return m_trusted; }

	protected:
		FileInfo MakeFileInfo(const std::string &path, FileInfo::FileType entryType, Time::DateTime modTime);
		FileInfo MakeFileInfo(const std::string &path, FileInfo::FileType entryType);

	private:
		std::string m_root;
		bool m_trusted;
	};

	class FileSourceFS : public FileSource {
	public:
		explicit FileSourceFS(const std::string &root, bool trusted = false);
		~FileSourceFS();

		virtual FileInfo Lookup(const std::string &path);
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

		bool MakeDirectory(const std::string &path);

		enum WriteFlags {
			WRITE_TEXT = 1
		};

		// similar to fopen(path, "rb")
		FILE *OpenReadStream(const std::string &path);
		// similar to fopen(path, "wb")
		FILE *OpenWriteStream(const std::string &path, int flags = 0);
		bool RemoveFile(const std::string &relativePath);
		bool IsChildOfRoot(const std::string &path);
	};

	class FileSourceUnion : public FileSource {
	public:
		FileSourceUnion();
		~FileSourceUnion();

		// add and remove sources
		// note: order is important. The array of sources works like a PATH array:
		// that is, earlier sources take priority over later sources
		void PrependSource(FileSource *fs);
		void AppendSource(FileSource *fs);
		void RemoveSource(FileSource *fs);

		virtual FileInfo Lookup(const std::string &path);
		std::vector<FileInfo> LookupAll(const std::string &path);
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

	private:
		std::vector<FileSource *> m_sources;
	};

} // namespace FileSystem

inline std::string FileSystem::FileInfo::GetAbsoluteDir() const
{
	return JoinPath(m_source->GetRoot(), GetDir());
}

inline std::string FileSystem::FileInfo::GetAbsolutePath() const
{
	return JoinPath(m_source->GetRoot(), GetPath());
}

inline RefCountedPtr<FileSystem::FileData> FileSystem::FileInfo::Read() const
{
	return m_source->ReadFile(m_path);
}

#endif
