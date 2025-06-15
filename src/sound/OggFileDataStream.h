// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __OGG_FILE_DATA_STREAM_H
#define __OGG_FILE_DATA_STREAM_H

#include "FileSystem.h"

#include "vorbis/vorbisfile.h"

namespace Sound
{

class OggFileDataStream {
public:
    OggFileDataStream() :
        m_cursor(nullptr) {}
    explicit OggFileDataStream(const RefCountedPtr<FileSystem::FileData> &data) :
        m_data(data),
        m_cursor(data->GetData()) { assert(data); }

    void Reset()
    {
        m_data.Reset();
        m_cursor = nullptr;
    }

    void Reset(const RefCountedPtr<FileSystem::FileData> &data)
    {
        assert(data);
        m_data = data;
        m_cursor = m_data->GetData();
    }

    size_t read(char *buf, size_t sz, int n)
    {
        assert(n >= 0);
        ptrdiff_t offset = tell();
        // clamp to available data
        n = std::min(n, int((m_data->GetSize() - offset) / sz));
        size_t fullsize = sz * n;
        assert(offset + fullsize <= m_data->GetSize());
        memcpy(buf, m_cursor, fullsize);
        m_cursor += fullsize;
        return n;
    }

    int seek(ogg_int64_t offset, int whence)
    {
        switch (whence) {
        case SEEK_SET: m_cursor = m_data->GetData() + offset; break;
        case SEEK_END: m_cursor = m_data->GetData() + (m_data->GetSize() + offset); break;
        case SEEK_CUR: m_cursor += offset; break;
        default: return -1;
        }
        return 0;
    }

    long tell()
    {
        assert(m_data && m_cursor);
        return long(m_cursor - m_data->GetData());
    }

    int close()
    {
        if (m_data) {
            m_data.Reset();
            m_cursor = nullptr;
            return 0;
        } else {
            return -1;
        }
    }

private:
    static size_t ov_callback_read(void *buf, size_t sz, size_t n, void *stream)
    {
        return reinterpret_cast<OggFileDataStream *>(stream)->read(reinterpret_cast<char *>(buf), sz, n);
    }
    static int ov_callback_seek(void *stream, ogg_int64_t offset, int whence)
    {
        return reinterpret_cast<OggFileDataStream *>(stream)->seek(offset, whence);
    }
    static long ov_callback_tell(void *stream)
    {
        return reinterpret_cast<OggFileDataStream *>(stream)->tell();
    }
    static int ov_callback_close(void *stream)
    {
        return reinterpret_cast<OggFileDataStream *>(stream)->close();
    }

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
