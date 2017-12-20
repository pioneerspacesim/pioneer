// Copyright © 2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GZIP_FORMAT_H
#define GZIP_FORMAT_H

#include <string>

namespace gzip {
	struct GZipException {};
	struct DecompressionFailedException : public GZipException {};
	struct CompressionFailedException : public GZipException {};

	// Checks if the data block could plausibly be a GZip file.
	// This really just checks for the magic GZip bytes and a basic length check.
	bool IsGZipFormat(const unsigned char *data, size_t length);

	// Decompresses a compressed block.
	// If the input starts with GZip header marker bytes then it will interpret the input as GZip data.
	// If the input does not start with the GZip header marker then it will assume it's a plain DEFLATE
	// input and decompress it as such.
	std::string DecompressDeflateOrGZip(const unsigned char *data, size_t length);

	// Decompress GZip format data.
	// This tries to follow RFC 1952 (GZip format).
	// If the input fails format or CRC checks then it will throw an exception.
	std::string DecompressGZip(const unsigned char *data, size_t length);

	// Wrapper for miniz inflate function; assumes no header.
	std::string DecompressRawDeflate(const unsigned char *data, size_t length);

	// Compresses a block of data and adds a GZip format header.
	// This tries to follow RFC 1952 (GZip format).
	// If compression fails it throws an exception.
	// Parameter 'inner_file_name' is the name written in the GZip header as the file name of the compressed block.
	std::string CompressGZip(const std::string &data, const std::string &inner_file_name);
}

#endif
