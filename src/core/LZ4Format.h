// Copyright © 2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lz4 {

	struct DecompressionFailedException : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
	struct CompressionFailedException : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	// Checks if the data block could plausibly be a lz4 file.
	// This really just checks for the magic lz4 bytes and a basic length check.
	bool IsLZ4Format(const char *data, size_t length);

	// Decompress lz4 format data.
	// If the input fails format checks or checksum then it will throw an exception.
	std::string DecompressLZ4(const std::string_view data);

	// Compresses a block of data according to the lz4 framing format.
	// If compression fails it throws an exception.
	// lz4_speed is the compression preset; 0 = default compression, 3-12 = HC compression
	std::string CompressLZ4(const std::string_view data, const int lz4_preset);
} // namespace lz4
