#include "OggFileDataStream.h"

Sound::OggFileDataStream::OggFileDataStream() :
	m_cursor(nullptr)
{
}

Sound::OggFileDataStream::OggFileDataStream(const RefCountedPtr<FileSystem::FileData> &data) :
	m_data(data),
	m_cursor(data->GetData()) { assert(data); }

void Sound::OggFileDataStream::Reset()
{
	m_data.Reset();
	m_cursor = nullptr;
}

void Sound::OggFileDataStream::Reset(const RefCountedPtr<FileSystem::FileData> &data)
{
	assert(data);
	m_data = data;
	m_cursor = m_data->GetData();
}

size_t Sound::OggFileDataStream::read(char *buf, size_t sz, int n)
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

int Sound::OggFileDataStream::seek(ogg_int64_t offset, int whence)
{
	switch (whence) {
	case SEEK_SET: m_cursor = m_data->GetData() + offset; break;
	case SEEK_END: m_cursor = m_data->GetData() + (m_data->GetSize() + offset); break;
	case SEEK_CUR: m_cursor += offset; break;
	default: return -1;
	}
	return 0;
}

long Sound::OggFileDataStream::tell()
{
	assert(m_data && m_cursor);
	return long(m_cursor - m_data->GetData());
}

int Sound::OggFileDataStream::close()
{
	if (m_data) {
		m_data.Reset();
		m_cursor = nullptr;
		return 0;
	} else {
		return -1;
	}
}

size_t Sound::OggFileDataStream::ov_callback_read(void *buf, size_t sz, size_t n, void *stream)
{
	return reinterpret_cast<OggFileDataStream *>(stream)->read(reinterpret_cast<char *>(buf), sz, n);
}

int Sound::OggFileDataStream::ov_callback_seek(void *stream, ogg_int64_t offset, int whence)
{
	return reinterpret_cast<OggFileDataStream *>(stream)->seek(offset, whence);
}

long Sound::OggFileDataStream::ov_callback_tell(void *stream)
{
	return reinterpret_cast<OggFileDataStream *>(stream)->tell();
}

int Sound::OggFileDataStream::ov_callback_close(void *stream)
{
	return reinterpret_cast<OggFileDataStream *>(stream)->close();
}
