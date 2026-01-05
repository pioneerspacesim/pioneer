// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __OGG_FILE_DATA_STREAM_H
#define __OGG_FILE_DATA_STREAM_H

#include "FileSystem.h"

#include "vorbis/vorbisfile.h"

namespace Sound {

	class OggFileDataStream {
	public:
		OggFileDataStream();
		explicit OggFileDataStream(const RefCountedPtr<FileSystem::FileData> &data);

		void Reset();
		void Reset(const RefCountedPtr<FileSystem::FileData> &data);

		size_t read(char *buf, size_t sz, int n);
		int seek(ogg_int64_t offset, int whence);
		long tell();
		int close();

	private:
		static size_t ov_callback_read(void *buf, size_t sz, size_t n, void *stream);
		static int ov_callback_seek(void *stream, ogg_int64_t offset, int whence);
		static long ov_callback_tell(void *stream);
		static int ov_callback_close(void *stream);

		RefCountedPtr<FileSystem::FileData> m_data;
		const char *m_cursor;

	public:
		static inline ov_callbacks CALLBACKS{
			&OggFileDataStream::ov_callback_read,
			&OggFileDataStream::ov_callback_seek,
			&OggFileDataStream::ov_callback_close,
			&OggFileDataStream::ov_callback_tell
		};
	};

} // namespace Sound

#endif // __OGG_FILE_DATA_STREAM_H
