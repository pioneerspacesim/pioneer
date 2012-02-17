#include "libs.h"
#include "IniConfig.h"
#include "FileSystem.h"

static const char *buf_find_newline(const char *str, const char *strend)
{
	while ((str != strend) && (*str != '\n') && (*str != '\r')) {
		++str;
	}
	return str;
}

static const char *buf_find_chr(const char *str, const char *strend, char c)
{
	while ((str != strend) && (*str != c)) { ++str; }
	return str;
}

static const char *buf_skip_space(const char *str, const char *strend)
{
	while ((str != strend) && isspace(*str)) { ++str; }
	return str;
}

static const char *buf_rskip_space(const char *str, const char *strend)
{
	while ((str != strend) && isspace(*--str));
	return strend;
}

void IniConfig::Load()
{
	RefCountedPtr<FileSystem::FileData> data = FileSystem::rawFileSystem.ReadFile(m_filename);
	if (data) {
		Load(*data);
	}
}

void IniConfig::Load(const FileSystem::FileData &data)
{
	const char *buf = reinterpret_cast<const char*>(data.GetData());
	const char *bufend = buf + data.GetSize();

	while (buf != bufend) {
		// skip space at the beginning of the line
		buf = buf_skip_space(buf, bufend);
		const char *line = buf;
		// find the end of the line
		buf = buf_find_newline(buf, bufend);

		// if the line is a comment, skip it
		if (line[0] == '#') continue;
		const char *kend = buf_find_chr(line, buf, '=');
		// if there's no '=' sign, skip the line
		if (kend == buf) continue;

		// strip whitespace
		const char *vbegin = buf_skip_space(kend + 1, buf);
		const char *vend = buf_rskip_space(vbegin, buf);
		kend = buf_rskip_space(line, kend);

		const std::string key = std::string(line, kend - line);
		const std::string val = std::string(vbegin, vend - vbegin);
		(*this)[key] = val;
	}
}

bool IniConfig::Save()
{
	FILE *f = fopen(m_filename.c_str(), "w");
	if (!f)
		// XXX do something useful here
		return false;

	for (std::map<std::string, std::string>::const_iterator i = begin(); i!=end(); ++i) {
		if ((*i).second != "") fprintf(f, "%s=%s\n", (*i).first.c_str(), (*i).second.c_str());
	}

	fclose(f);

	return true;
}
