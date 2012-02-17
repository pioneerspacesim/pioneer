#include "libs.h"
#include "IniConfig.h"
#include "FileSystem.h"
#include "StringRange.h"

void IniConfig::Load()
{
	RefCountedPtr<FileSystem::FileData> data = FileSystem::rawFileSystem.ReadFile(m_filename);
	if (data) {
		Load(*data);
	}
}

void IniConfig::Load(const FileSystem::FileData &data)
{
	StringRange buffer(
		reinterpret_cast<const char*>(data.GetData()),
		reinterpret_cast<const char*>(data.GetData()) + data.GetSize());

	while (!buffer.Empty()) {
		// skip space at the beginning of the line
		buffer.begin = buffer.SkipSpace();
		// find the end of the line
		const char *eol = buffer.FindNewline();
		StringRange line(buffer.begin, eol);
		buffer.begin = eol;

		// if the line is a comment, skip it
		if (line[0] == '#') continue;
		const char *kend = line.FindChar('=');
		// if there's no '=' sign, skip the line
		if (kend == eol) continue;

		StringRange key(line.begin, kend);
		StringRange value(kend + 1, line.end);
		// strip whitespace
		key.end = key.RSkipSpace();
		value.begin = value.SkipSpace();
		value.end = value.RSkipSpace();

		(*this)[std::string(key.begin, key.Length())] = std::string(value.begin, value.Length());
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
