#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <string>
#include <vector>
#include <deque>

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

	const std::string &GetUserDir();
	const std::string &GetDataDir();

	class FileSource;

	class FileData : public RefCounted {
	public:
		virtual ~FileData() {}

		const FileInfo &GetInfo() const { return m_info; }
		size_t GetSize() const { return m_size; }
		const unsigned char *GetData() const { assert(m_info.IsFile()); return m_data; }

	protected:
		FileData(const FileInfo &info, size_t size, unsigned char *data):
			m_info(info), m_data(data), m_size(size) {}
		FileData(const FileInfo &info): m_info(info), m_data(0), m_size(0) {}

	private:
		FileInfo m_info;
		unsigned char *m_data;
		size_t m_size;
	};

	class FileInfo {
		friend class FileSource;
	public:
		enum FileType {
			FT_NON_EXISTENT,
			FT_FILE,
			FT_DIR,
			FT_SPECIAL
		};

		bool Exists() const { return (m_fileType != FT_NON_EXISTENT); }
		bool IsDir() const { return (m_fileType == FT_DIR); }
		bool IsFile() const { return (m_fileType == FT_FILE); }

		std::string GetName() const { return m_path.substr(m_dirLen); }
		std::string GetDir() const { return m_path.substr(0, m_dirLen); }
		const std::string &GetPath() const { return m_path; }
		const std::string &GetSourcePath() const { return m_fileSource->GetSourcePath(); }

		const FileSource &GetSource() const { return *m_fileSource; }

		RefCountedPtr<FileData> Read(const std::string &path) { return m_source->Read(m_path); }

	protected:
		FileInfo(const FileSource *source, const std::string &path, FileType type);

	private:
		const FileSource *m_source;
		std::string m_path;
		int m_dirLen;
		FileType m_fileType;
	};

	class FileSource {
	public:
		FileSource(const std::string &sourcePath);
		virtual ~FileSource();

		const std::string &GetSourcePath() const { return m_sourcePath; }

		virtual RefCountedPtr<FileData> Read(const std::string &path) = 0;
		virtual FileInfo Lookup(const std::string &path) = 0;
		virtual void ReadDirectory(const std::string &path, std::vector<FileInfo> &output) = 0;

	protected:
		FileInfo MakeFileInfo(const std::string &path, FileInfo::FileType entryType);

	private:
		std::string m_sourcePath;
	};

	class FileSystem {
	public:
		FileSystem();
		~FileSystem();

		void AddSource(std::auto_ptr<FileSource> source);

		RefCountedPtr<FileData> Read(const std::string &path);
		FileInfo Lookup(const std::string &path);
		void ReadDirectory(const std::string &path, std::vector<FileInfo> &output);

	private:
		std::vector<FileSource*> m_sources;
	};

	class FileEnumerator {
	public:
		enum Flags {
			Recurse            = 1,
			IncludeDuplicates  = 2,
			IncludeDirectories = 4,
			OnlyDirectories    = 4|8
		};

		explicit FileEnumerator(const FileSystem &fs, int flags = 0);
		explicit FileEnumerator(const FileSystem &fs, const std::string &path, int flags = 0);
		~FileEnumerator();

		bool Finished() const;
		void Next();
		const FilePath &Current() const { return m_queue.front(); }

	private:
		std::deque<FileInfo> m_queue;
	};

}

#endif
