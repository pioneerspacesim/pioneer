// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "IniConfig.h"
#include "FileSystem.h"
#include "StringRange.h"

void IniConfig::Read(FileSystem::FileSource &fs, const std::string &path)
{
	RefCountedPtr<FileSystem::FileData> data = fs.ReadFile(path);
	if (data) Read(*data);
}

void IniConfig::Read(const FileSystem::FileData &data)
{
	StringRange buffer = data.AsStringRange();
	buffer = buffer.StripUTF8BOM();

	while (!buffer.Empty()) {
		StringRange line = buffer.ReadLine().StripSpace();

		// if the line is a comment, skip it
		if (line.Empty() || (line[0] == '#')) continue;
		const char *kend = line.FindChar('=');
		// if there's no '=' sign, skip the line
		if (kend == line.end) {
			fprintf(stderr, "WARNING: ignoring invalid line in config file:\n   '%.*s'\n", int(line.Size()), line.begin);
			continue;
		}

		StringRange key(line.begin, kend);
		StringRange value(kend + 1, line.end);
		// strip whitespace
		key.end = key.RFindNonSpace();
		value = value.StripSpace();

		m_map[key.ToString()] = value.ToString();
	}
}

bool IniConfig::Write(FileSystem::FileSourceFS &fs, const std::string &path)
{
	FILE *f = fs.OpenWriteStream(path, FileSystem::FileSourceFS::WRITE_TEXT);
	if (!f) {
		fprintf(stderr, "Could not write config file '%s'\n", FileSystem::JoinPath(fs.GetRoot(), path).c_str());
		return false;
	}
	for (std::map<std::string, std::string>::const_iterator i = m_map.begin(); i != m_map.end(); ++i) {
		if (!(*i).second.empty())
			fprintf(f, "%s=%s\n", (*i).first.c_str(), (*i).second.c_str());
	}
	fclose(f);
	return true;
}
