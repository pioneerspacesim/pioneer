#ifdef _WIN32

#ifndef _WIN32_TEXTUTILS_H
#define _WIN32_TEXTUTILS_H

#include <windows.h>

static std::wstring transcode_utf8_to_utf16(const char *s, size_t nbytes)
{
	std::wstring buf(nbytes, L'x');
	int reqchars = MultiByteToWideChar(CP_UTF8, 0, s, nbytes, &buf[0], buf.size());
	if (!reqchars) { fprintf(stderr, "failed to transcode UTF-8 to UTF-16\n"); abort(); }
	buf.resize(reqchars);
	return buf;
}

static std::wstring transcode_utf8_to_utf16(const std::string &s)
{
	return transcode_utf8_to_utf16(s.c_str(), s.size());
}

static std::string transcode_utf16_to_utf8(const wchar_t *s, size_t nchars)
{
	std::string buf(nchars * 2, 'x');
	int reqbytes = WideCharToMultiByte(CP_UTF8, 0, s, nchars, &buf[0], buf.size(), 0, 0);
	if (!reqbytes) { fprintf(stderr, "failed to transcode UTF-16 to UTF-8\n"); abort(); }
	buf.resize(reqbytes);
	return buf;
}

static std::string transcode_utf16_to_utf8(const std::wstring &s)
{
	return transcode_utf16_to_utf8(s.c_str(), s.size());
}

#endif // _WIN32_TEXTUTILS_H

#endif // _WIN32
