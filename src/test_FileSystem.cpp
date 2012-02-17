#include "FileSystem.h"
#include <cstdio>

static const char *ftype_name(const FileSystem::FileInfo &info) {
	if (info.IsDir()) { return "directory"; }
	else if (info.IsFile()) { return "file"; }
	else if (info.Exists()) { return "special"; }
	else { return "non-existent"; }
}

void test_filesystem()
{
	using namespace FileSystem;

	printf("data dir is '%s'\n", FileSystem::GetDataDir().c_str());
	printf("user dir is '%s'\n", FileSystem::GetUserDir().c_str());

	FileSourceFS fsAppData(FileSystem::GetDataDir());
	FileSourceFS fsUserData(FileSystem::GetUserDir() + "/data");

	printf("data root is '%s'\n", fsAppData.GetSourcePath().c_str());
	printf("user root is '%s'\n", fsUserData.GetSourcePath().c_str());

	FileSourceUnion fs;
	fs.AppendSource(&fsUserData);
	fs.AppendSource(&fsAppData);

	FileInfo info = fsAppData.Lookup("models");
	printf("models is: '%s' (%s)\n", info.GetPath().c_str(), ftype_name(info));

	printf("enumerating models:\n");
	for (FileEnumerator files(fs, "models", FileEnumerator::Recurse | FileEnumerator::IncludeDirectories); !files.Finished(); files.Next()) {
		const FileInfo &info = files.Current();
		printf("  %s (%s)\n", info.GetPath().c_str(), ftype_name(info));
	}
}
