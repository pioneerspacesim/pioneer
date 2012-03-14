#include "FileSystem.h"
#include <cstdio>
#include <stdexcept>

static const char *ftype_name(const FileSystem::FileInfo &info) {
	if (info.IsDir()) { return "directory"; }
	else if (info.IsFile()) { return "file"; }
	else if (info.Exists()) { return "special"; }
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

void test_filesystem()
{
	using namespace FileSystem;

	test_normpath();

	printf("data dir is '%s'\n", FileSystem::GetDataDir().c_str());
	printf("user dir is '%s'\n", FileSystem::GetUserDir().c_str());

	FileSourceFS fsAppData(FileSystem::GetDataDir());
	FileSourceFS fsUserData(FileSystem::GetUserDir() + "/data");

	printf("data root is '%s'\n", fsAppData.GetRoot().c_str());
	printf("user root is '%s'\n", fsUserData.GetRoot().c_str());

	FileSourceUnion fs;
	fs.AppendSource(&fsUserData);
	fs.AppendSource(&fsAppData);

	FileInfo info = fsAppData.Lookup("models");
	printf("models is: '%s' (%s)\n", info.GetPath().c_str(), ftype_name(info));

#if 0
	printf("enumerating models:\n");
	for (FileEnumerator files(fs, "models", FileEnumerator::Recurse | FileEnumerator::IncludeDirectories); !files.Finished(); files.Next()) {
		const FileInfo &info = files.Current();
		printf("  %s (%s)\n", info.GetPath().c_str(), ftype_name(info));
	}
#endif
}
