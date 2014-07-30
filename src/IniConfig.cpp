// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "IniConfig.h"
#include "FileSystem.h"
#include "StringRange.h"
#include "utils.h"
#include <cstdlib>
#include <sstream>

void IniConfig::SetInt(const std::string &section, const std::string &key, int val)
{
	std::ostringstream ss;
	ss << val;
	SetString(section, key, ss.str());
}

void IniConfig::SetFloat(const std::string &section, const std::string &key, float val)
{
	std::ostringstream ss;
	ss << val;
	SetString(section, key, ss.str());
}

void IniConfig::SetString(const std::string &section, const std::string &key, const std::string &val)
{
	m_map[section][key] = val;
}

int IniConfig::Int(const std::string &section, const std::string &key, int defval) const
{
	SectionMapType::const_iterator secIt = m_map.find(section);
	if (secIt == m_map.end()) return defval;
	MapType::const_iterator it = secIt->second.find(key);
	if (it == secIt->second.end()) return defval;

	const StringRange val = StringRange(it->second.c_str(), it->second.size()).StripSpace();
	if (val.Empty()) return defval;
	char *end = 0;
	long x = strtol(val.begin, &end, 10);
	if (end != val.end) return defval;
	return int(x);
}

float IniConfig::Float(const std::string &section, const std::string &key, float defval) const
{
	SectionMapType::const_iterator secIt = m_map.find(section);
	if (secIt == m_map.end()) return defval;
	MapType::const_iterator it = secIt->second.find(key);
	if (it == secIt->second.end()) return defval;

	const StringRange val = StringRange(it->second.c_str(), it->second.size()).StripSpace();
	if (val.Empty()) return defval;
	char *end = 0;
	float x = strtod(val.begin, &end);
	if (end != val.end) return defval;
	return x;
}

std::string IniConfig::String(const std::string &section, const std::string &key, const std::string &defval) const
{
	SectionMapType::const_iterator secIt = m_map.find(section);
	if (secIt == m_map.end()) return defval;
	MapType::const_iterator it = secIt->second.find(key);
	if (it == secIt->second.end()) return defval;
	return it->second;
}

void IniConfig::Read(FileSystem::FileSource &fs, const std::string &path)
{
	RefCountedPtr<FileSystem::FileData> data = fs.ReadFile(path);
	if (data) Read(*data);
}

void IniConfig::Read(const FileSystem::FileData &data)
{
	StringRange buffer = data.AsStringRange();
	buffer = buffer.StripUTF8BOM();

	std::string section_name;
	MapType *section_map = 0;

	while (!buffer.Empty()) {
		StringRange line = buffer.ReadLine().StripSpace();

		// if the line is a comment, skip it
		if (line.Empty() || (line[0] == '#')) continue;

		// check for a section header
		if ((line.Size() >= 2) && (line[0] == '[') && (line.end[-1] == ']')) {
			++line.begin;
			--line.end;
			section_name = line.ToString();
			section_map = 0;
			continue;
		}

		const char *kend = line.FindChar('=');
		// if there's no '=' sign, skip the line
		if (kend == line.end) {
			Output("WARNING: ignoring invalid line in config file:\n   '%.*s'\n", int(line.Size()), line.begin);
			continue;
		}

		StringRange key(line.begin, kend);
		StringRange value(kend + 1, line.end);
		// strip whitespace
		key.end = key.RFindNonSpace();
		value = value.StripSpace();

		if (!section_map)
			section_map = &m_map[section_name];

		(*section_map)[key.ToString()] = value.ToString();
	}
}

bool IniConfig::Write(FileSystem::FileSourceFS &fs, const std::string &path)
{
	FILE *f = fs.OpenWriteStream(path, FileSystem::FileSourceFS::WRITE_TEXT);
	if (!f) {
		Output("Could not write config file '%s'\n", FileSystem::JoinPath(fs.GetRoot(), path).c_str());
		return false;
	}
	for (SectionMapType::const_iterator secIt = m_map.begin(); secIt != m_map.end(); ++secIt) {
		const MapType &map = secIt->second;
		if (map.empty())
			continue;

		if (!secIt->first.empty()) {
			fprintf(f, "\n[%s]\n", secIt->first.c_str());
		}

		for (MapType::const_iterator it = map.begin(); it != map.end(); ++it) {
			if (!it->second.empty())
				fprintf(f, "%s=%s\n", it->first.c_str(), it->second.c_str());
		}
	}
	fclose(f);
	return true;
}
