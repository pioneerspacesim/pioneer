///\file
/// Versionning stuff (class, free functions).
/// This reduces compile time by avoiding direct inclusion of VersionNumber.h.

#ifndef _VERSION_H
#define _VERSION_H

#include <string>

/// Represents a version with four integers, such as "1.0.2.14".
///\author Sukender
///\version 1, from PVLE code (GPL) with boost-related code removed. Notable missing methods are to/fromString().
class Version {		// : public boost::totally_ordered<Version>
public:
	/// Quality information on the version (alpha, beta, RC...).
	enum EVersionQuality {
		QUALITY_DEV,
		QUALITY_ALPHA,
		QUALITY_BETA,
		QUALITY_RELEASE_CANDIDATE,
		QUALITY_GAMMA = QUALITY_RELEASE_CANDIDATE,		// Alias
		QUALITY_STABLE,
		MAX_QUALITY
	};

	Version(unsigned int majorNumber_ = 0, unsigned int minorNumber_ = 0, unsigned int revision_ = 0, unsigned int build_ = 0, EVersionQuality quality_ = QUALITY_STABLE)
		: majorNumber(majorNumber_), minorNumber(minorNumber_), revision(revision_), build(build_), quality(quality_)
	{}
	Version(const Version & v) : majorNumber(v.majorNumber), minorNumber(v.minorNumber), revision(v.revision), build(v.build), quality(v.quality) {}

	/// Sets the version to be "0.0.0.0" and QUALITY_STABLE.
	void reset() { majorNumber = 0; minorNumber = 0; revision = 0; build = 0; quality = QUALITY_STABLE; }

	Version & operator=(const Version & v) {
		majorNumber = v.majorNumber;
		minorNumber = v.minorNumber;
		revision = v.revision;
		build = v.build;
		quality = v.quality;
		return *this;
	}
	/// Equality does not test \c quality.
	bool operator==(const Version & v) const { return majorNumber == v.majorNumber && minorNumber == v.minorNumber && revision == v.revision && build == v.build; }
	/// Less-than does not test \c quality.
	bool operator<(const Version & v) const {
		if (majorNumber != v.majorNumber) return majorNumber < v.majorNumber;
		if (minorNumber != v.minorNumber) return minorNumber < v.minorNumber;
		if (revision != v.revision) return revision < v.revision;
		return build < v.build;
	}

	unsigned int majorNumber;		///< Major (first) version number.
	unsigned int minorNumber;		///< Minor (second) version number.
	unsigned int revision;			///< Revision (third) number.
	unsigned int build;				///< Build (fourth) number.

	EVersionQuality quality;		///< Quality indicator.
};

/// Retreives current version.
/// Prefer calling this function rather than including VersioNumber.h, to avoid recompiling each time version changes.
Version getVersion();

/// Retreives current version name.
std::string getVersionName();

/// Retreives current revision string (Git HEAD SHA1).
std::string getRevision();

#endif /* _VERSION_H */
