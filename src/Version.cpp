#include <Version.h>
#include <VersionNumber.h>
#include <Config.h>

//const char * const VERSION_QUALITY_STRS[Version::MAX_QUALITY] = {
//	"development",
//	"alpha",
//	"beta",
//	"Release Candidate",
//	""
//};

const char * const VERSION_QUALITY_SHORT_STRS[Version::MAX_QUALITY] = {
	"dev",
	"a",
	"b",
	"RC",
	""
};

/// Returns quality enum from string representation.
inline Version::EVersionQuality qualityFromString(const std::string & quality) {		// quality is intentionally NOT a "const char * const", to ensure calling "==" will result in string comparison. BTW, I (Sukender) dislike strcmp() for readability reasons and will not use it.
	for(int i=0; i<Version::MAX_QUALITY; ++i) {
		if (quality == VERSION_QUALITY_SHORT_STRS[i]) return static_cast<Version::EVersionQuality>(i);
	}
	return Version::QUALITY_STABLE;
}

Version getVersion() {
	return Version(Pioneer_MAJOR_VERSION, Pioneer_MINOR_VERSION, Pioneer_PATCH_VERSION, Pioneer_BUILD_VERSION, qualityFromString(Pioneer_VERSION_QUALITY));
}

std::string getRevision() { return Pioneer_REVISION; }
