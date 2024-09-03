// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WIN32_TEXTUTILS_H
#define _WIN32_TEXTUTILS_H

#include <string>

std::wstring transcode_utf8_to_utf16(const char *s, size_t nbytes);
std::wstring transcode_utf8_to_utf16(const std::string &s);
std::string transcode_utf16_to_utf8(const wchar_t *s, size_t nchars);
std::string transcode_utf16_to_utf8(const std::wstring &s);

#endif // _WIN32_TEXTUTILS_H
