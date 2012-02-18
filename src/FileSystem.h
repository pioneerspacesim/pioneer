#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "RefCounted.h"
#include "StringRange.h"
#include <string>
#include <vector>
#include <deque>
#include <memory>

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
	extern FileSourceFS rawFileSystem;

	std::string GetUserDir(const char *subdir = 0);
	std::string GetDataDir(const char *subdir = 0);

	std::string AbsolutePath(const std::string &path);
	std::string JoinPath(const std::string &a, const std::string &b);

	class FileInfo {
		friend class FileSource;
	public:
		enum FileType {
			// note: order here affects sort-order of FileInfo
			FT_DIR,
			FT_FILE,
			FT_NON_EXISTENT,
			FT_SPECIAL
		};

		bool Exists() const { return (m_type != FT_NON_EXISTENT); }
		bool IsDir() const { return (m_type == FT_DIR); }
		bool IsFile() const { return (m_type == FT_FILE); }

		std::string GetName() const { return m_path.substr(m_dirLen); }
		std::string GetDir() const { return m_path.substr(0, m_dirLen); }
		const std::string &GetPath() const { return m_path; }
		const std::string &GetSourcePath() const;

		const FileSource &GetSource() const { return *m_source; }

		RefCountedPtr<FileData> Read(const std::string &path) const;

		friend bool operator==(const FileInfo &a, const FileInfo &b)
		{ return (a.m_source == b.m_source && a.m_type == b.m_type && a.m_path == b.m_path); }
		friend bool operator!=(const FileInfo &a, const FileInfo &b)
		{ return (a.m_source != b.m_source || a.m_type != b.m_type || a.m_path != b.m_path); }
		friend bool operator< (const FileInfo &a, const FileInfo &b)
		{
			int c = a.m_path.compare(b.m_path);
			if (c != 0) { return (c < 0); }
			if (a.m_type != b.m_type) { return (a.m_type < b.m_type); }
			return (a.m_source < b.m_source);
		}
		friend bool operator<=(const FileInfo &a, const FileInfo &b)
		{
			int c = a.m_path.compare(b.m_path);
			if (c != 0) { return (c < 0); }
			if (a.m_type != b.m_type) { return (a.m_type < b.m_type); }
			return (a.m_source <= b.m_source);
		}
		friend bool operator> (const FileInfo &a, const FileInfo &b) { return (b < a); }
		friend bool operator>=(const FileInfo &a, const FileInfo &b) { return (b <= a); }

	protected:
		FileInfo(FileSource *source, const std::string &path, FileType type);

	private:
		FileSource *m_source;
		std::string m_path;
		int m_dirLen;
		FileType m_type;
	};

	class FileData : public RefCounted {
	public:
		virtual ~FileData() {}

		const FileInfo &GetInfo() const { return m_info; }
		size_t GetSize() const { return m_size; }
		const char *GetData() const { assert(m_info.IsFile()); return m_data; }
		StringRange AsStringRange() const { return StringRange(m_data, m_data + m_size); }

	protected:
		FileData(const FileInfo &info, size_t size, char *data):
			m_info(info), m_data(data), m_size(size) {}
		FileData(const FileInfo &info): m_info(info), m_data(0), m_size(0) {}

		FileInfo m_info;
		char *m_data;
		size_t m_size;
	};

	class FileDataMalloc : public FileData {
	public:
		FileDataMalloc(const FileInfo &info, size_t size):
			FileData(info, size, reinterpret_cast<char*>(std::malloc(size))) {}
		FileDataMalloc(const FileInfo &info, size_t size, char *data):
			FileData(info, size, data) {}
		virtual ~FileDataMalloc() { std::free(m_data); }
	};

	class FileSource {
	public:
		explicit FileSource(const std::string &sourcePath): m_sourcePath(sourcePath) {}
		virtual ~FileSource() {}

		const std::string &GetSourcePath() const { return m_sourcePath; }

		virtual FileInfo Lookup(const std::string &path) = 0;
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path) = 0;
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output) = 0;

	protected:
		FileInfo MakeFileInfo(const std::string &path, FileInfo::FileType entryType);

	private:
		std::string m_sourcePath;
	};

	class FileSourceFS : public FileSource {
	public:
		explicit FileSourceFS(const std::string &root);
		~FileSourceFS();

		virtual FileInfo Lookup(const std::string &path);
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

		bool MakeDirectory(const std::string &path);

	private:
		bool MakeDirectoryRaw(const std::string &path);
	};

	class FileSourceUnion : public FileSource {
	public:
		FileSourceUnion();
		~FileSourceUnion();

		void PrependSource(FileSource *fs);
		void AppendSource(FileSource *fs);
		void RemoveSource(FileSource *fs);

		virtual FileInfo Lookup(const std::string &path);
		virtual RefCountedPtr<FileData> ReadFile(const std::string &path);
		virtual bool ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

	private:
		std::vector<FileSource*> m_sources;
	};

	class FileEnumerator {
	public:
		enum Flags {
			IncludeDirectories = 1,
			OnlyDirectories    = 1|2, // ExcludeFiles = 2
			Recurse            = 1|4  // can't recurse without including directories (sorry)
		};

		explicit FileEnumerator(FileSource &fs, int flags = 0);
		explicit FileEnumerator(FileSource &fs, const std::string &path, int flags = 0);
		~FileEnumerator();

		bool Finished() const { return m_queue.empty(); }
		void Next() { Next(m_flags); }
		const FileInfo &Current() const { return m_queue.front(); }

	private:
		static const int ExcludeFiles = 2;
		void Next(int flags);
		void Init(const std::string &path);
		FileSource *m_source;
		std::deque<FileInfo> m_queue;
		int m_flags;
	};

} // namespace FileSystem

inline const std::string &FileSystem::FileInfo::GetSourcePath() const
{ return m_source->GetSourcePath(); }
inline RefCountedPtr<FileSystem::FileData> FileSystem::FileInfo::Read(const std::string &path) const
{ return m_source->ReadFile(m_path); }

#endif
