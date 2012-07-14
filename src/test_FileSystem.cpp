#include "FileSystem.h"
#include "FileSourceZip.h"
#include "utils.h"
#include <cstdio>
#include <stdexcept>

static const char *ftype_name(const FileSystem::FileInfo &info) {
	if (info.IsDir()) { return "directory"; }
	else if (info.IsFile()) { return "file"; }
	else if (info.IsSpecial()) { return "special"; }
	else { return "non-existent"; }
}

void test_normpath()
{
	using namespace FileSystem;
	const char *TEST_PATHS[] = {
		"a/b/c",
		"a/b/c/",
		"/a/b/c",
		"a/b/../c",
		"..",
		".",
		"./",
		"a/..",
		"a/b/./.././c/../",
		"a/b/c/d/../../../../",
		"a/b/c/d/../../../../../",
		"/a/b",
		"/a/b/",
		0
	};
	for (const char **path = TEST_PATHS; *path; ++path) {
		try {
			printf("'%s' -> '%s'\n", *path, NormalisePath(*path).c_str());
		} catch (std::invalid_argument) {
			printf("'%s' -> invalid\n", *path);
		}
	}

	printf("'/' + '/a/b/' -> '%s'\n", JoinPathBelow("/", "/a/b/").c_str());
}

void test_enum_models(FileSystem::FileSource &fs)
{
	using namespace FileSystem;

	printf("enumerating models and sub-models:\n");
	FileEnumerator files(fs, FileEnumerator::Recurse | FileEnumerator::IncludeDirs);
	files.AddSearchRoot("models");
	files.AddSearchRoot("sub_models");
	while (!files.Finished()) {
		const FileInfo &fi = files.Current();
		if (fi.IsDir() || ends_with(fi.GetPath(), ".lua")) {
			printf("  %s (%s) (%s)\n", fi.GetPath().c_str(), ftype_name(fi), fi.GetSource().GetRoot().c_str());
		}
		files.Next();
	}
}

void test_filesystem()
{
	using namespace FileSystem;

	test_normpath();

	printf("data dir is '%s'\n", FileSystem::GetDataDir().c_str());
	printf("user dir is '%s'\n", FileSystem::GetUserDir().c_str());

	FileSourceFS fsAppData(FileSystem::GetDataDir());
	FileSourceFS fsUserData(FileSystem::JoinPath(FileSystem::GetUserDir(), "/data"));
	//FileSourceZip fsZip("/home/jpab/.pioneer/mods/swapships.zip");

	printf("data root is '%s'\n", fsAppData.GetRoot().c_str());
	printf("user root is '%s'\n", fsUserData.GetRoot().c_str());
	//printf("zip root is '%s'\n", fsZip.GetRoot().c_str());

	FileSourceUnion fs;
	//fs.AppendSource(&fsZip);
	//printf("Just zip:\n");
	//test_enum_models(fs);

	fs.AppendSource(&fsUserData);
	fs.AppendSource(&fsAppData);
	test_enum_models(fs);

	//printf("With zip:\n");
	//test_enum_models(fs);

	//fs.RemoveSource(&fsZip);
	//printf("Just data:\n");
	test_enum_models(fs);
}
