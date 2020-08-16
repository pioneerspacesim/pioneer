#include "GZipFormat.h"
#include <cstdint>
#include <cstring>
#include <string>

extern "C" {
#include <miniz/miniz.h>
}

namespace {
	enum GZipFlags {
		FLAG_TEXT = 1,
		FLAG_HCRC = 2, // Header CRC included
		FLAG_EXTRA = 4,
		FLAG_NAME = 8,
		FLAG_COMMENT = 16,

		FLAGS_ALL = (FLAG_TEXT | FLAG_HCRC | FLAG_EXTRA | FLAG_NAME | FLAG_COMMENT),
	};

	enum GZipCompressionModes {
		CM_DEFLATE = 8, // The only one defined in the RFC (and the one we need).
	};

	enum GZipHeaderSizes {
		BASE_HEADER_SIZE = 10,
		BASE_FOOTER_SIZE = 8,
	};

	// Streaming output function for tdefl_compress_mem_to_output.
	static mz_bool PutBytesToString(const void *buf, int len, void *user)
	{
		std::string *out = static_cast<std::string *>(user);
		out->append(static_cast<const char *>(buf), len);
		return MZ_TRUE;
	}

	static uint32_t ReadLE32(const unsigned char *data)
	{
		return (uint32_t(data[0]) << 0) |
			(uint32_t(data[1]) << 8) |
			(uint32_t(data[2]) << 16) |
			(uint32_t(data[3]) << 24);
	}

	static void WriteLE32(unsigned char *out, uint32_t value)
	{
		out[0] = (value >> 0) & 0xffu;
		out[1] = (value >> 8) & 0xffu;
		out[2] = (value >> 16) & 0xffu;
		out[3] = (value >> 24) & 0xffu;
	}
} // namespace

bool gzip::IsGZipFormat(const unsigned char *data, size_t length)
{
	// (This assumes it's possible to provide 0 bytes of compressed data, which is probably not true).
	if (length < BASE_HEADER_SIZE + BASE_FOOTER_SIZE) {
		return false;
	}
	if (data[0] != 0x1fu || data[1] != 0x8bu) {
		return false;
	}
	return true;
}

std::string gzip::DecompressDeflateOrGZip(const unsigned char *data, size_t length)
{
	assert(data != nullptr);
	if (gzip::IsGZipFormat(data, length)) {
		return gzip::DecompressGZip(data, length);
	} else {
		// No GZip header. Assume raw DEFLATE data (which is what Pioneer used to save).
		return gzip::DecompressRawDeflate(data, length);
	}
}

std::string gzip::DecompressGZip(const unsigned char *data, size_t length)
{
	assert(data != nullptr);
	assert(length >= BASE_HEADER_SIZE + BASE_FOOTER_SIZE);

	const unsigned char *at_header = data;
	const unsigned char *at_footer = data + (length - BASE_FOOTER_SIZE);

	// We only know about DEFLATE.
	if (at_header[2] != CM_DEFLATE) {
		throw gzip::DecompressionFailedException();
	}

	int gzip_flags = at_header[3];
	// There are not supposed to be any unknown flags!
	if (gzip_flags & ~FLAGS_ALL) {
		throw gzip::DecompressionFailedException();
	}

	const unsigned char *at_data = at_header + BASE_HEADER_SIZE;
	assert(at_data <= at_footer);
	size_t data_length = length - BASE_HEADER_SIZE + BASE_FOOTER_SIZE;

	if (gzip_flags & FLAG_EXTRA) {
		if (data_length < 2) {
			throw gzip::DecompressionFailedException();
		}
		size_t xlen = uint8_t(at_data[0]) | (uint8_t(at_data[1]) << 8);
		xlen += 2; // Add the two bytes for the length itself.
		if (data_length < xlen) {
			throw gzip::DecompressionFailedException();
		}
		at_data += xlen;
		assert(at_data <= at_footer);
		data_length = at_footer - at_data;
	}

	if (gzip_flags & FLAG_NAME) {
		const unsigned char *name_end = static_cast<const unsigned char *>(std::memchr(at_data, 0, data_length));
		if (!name_end) {
			throw gzip::DecompressionFailedException();
		}
		at_data = name_end + 1; // +1 to skip the null terminator.
		assert(at_data <= at_footer);
		data_length = at_footer - at_data;
	}

	if (gzip_flags & FLAG_COMMENT) {
		const unsigned char *comment_end = static_cast<const unsigned char *>(std::memchr(at_data, 0, data_length));
		if (!comment_end) {
			throw gzip::DecompressionFailedException();
		}
		at_data = comment_end + 1; // +1 to skip the null terminator.
		assert(at_data <= at_footer);
		data_length = at_footer - at_data;
	}

	if (gzip_flags & FLAG_HCRC) {
		if (data_length < 2) {
			throw gzip::DecompressionFailedException();
		}
		uint32_t true_crc = mz_crc32(MZ_CRC32_INIT, at_header, (at_data - at_header));
		true_crc &= 0xffffu; // Only care about the bottom 16 bits.
		uint32_t file_crc = uint8_t(at_data[0]) | (uint8_t(at_data[1]) << 8);
		if (true_crc != file_crc) {
			throw gzip::DecompressionFailedException();
		}
		at_data += 2;
		data_length -= 2;
		assert(at_data <= at_footer);
	}

	std::string out;

	assert(at_data + data_length == at_footer);
	bool inflate_success = tinfl_decompress_mem_to_callback(static_cast<const void *>(at_data), &data_length, &PutBytesToString, static_cast<void *>(&out), 0);
	if (!inflate_success) {
		throw gzip::DecompressionFailedException();
	}

	uint32_t true_crc = mz_crc32(MZ_CRC32_INIT, reinterpret_cast<const mz_uint8 *>(out.data()), out.size());
	uint32_t crc_from_file = ReadLE32(at_footer + 0);
	uint32_t size_from_header = ReadLE32(at_footer + 4);
	if (true_crc != crc_from_file) {
		throw gzip::DecompressionFailedException();
	}
	if (size_from_header != static_cast<uint32_t>(out.size())) {
		throw gzip::DecompressionFailedException();
	}

	return out;
}

std::string gzip::DecompressRawDeflate(const unsigned char *data, size_t length)
{
	assert(data != nullptr);
	std::string out;
	size_t in_size = length;
	bool success = tinfl_decompress_mem_to_callback(static_cast<const void *>(data), &in_size, &PutBytesToString, static_cast<void *>(&out), 0);
	if (!success) {
		throw gzip::DecompressionFailedException();
	}
	return out;
}

std::string gzip::CompressGZip(const std::string &data, const std::string &inner_file_name)
{
	std::string out;

	// The base GZip header.
	const unsigned char header_bytes[10] = { 31, 139, 8, FLAG_HCRC | FLAG_NAME, 0, 0, 0, 0, 0, 255 };
	out.append(reinterpret_cast<const char *>(header_bytes), sizeof(header_bytes));

	// Add inner file name, *including* null terminator (c_str() ensures that the data is null terminated).
	out.append(inner_file_name.c_str(), inner_file_name.size() + 1);

	// Add 16-bit header-CRC.
	uint32_t header_crc = mz_crc32(MZ_CRC32_INIT, reinterpret_cast<const mz_uint8 *>(out.data()), out.size());
	const unsigned char crc_buf[2] = {
		static_cast<unsigned char>((header_crc >> 0) & 0xffu),
		static_cast<unsigned char>((header_crc >> 8) & 0xffu),
	};
	out.append(reinterpret_cast<const char *>(crc_buf), sizeof(crc_buf));

	bool success = tdefl_compress_mem_to_output(data.data(), data.size(), &PutBytesToString, static_cast<void *>(&out), TDEFL_DEFAULT_MAX_PROBES);
	if (!success) {
		throw gzip::CompressionFailedException();
	}

	unsigned char footer_bytes[8];
	uint32_t data_crc = mz_crc32(MZ_CRC32_INIT, reinterpret_cast<const mz_uint8 *>(data.data()), data.size());
	WriteLE32(footer_bytes + 0, data_crc);
	// GZip specifies that size is written little-endian, modulo 2^32
	// (ie, if size is really > 2^32 we just chop off the high bits).
	WriteLE32(footer_bytes + 4, data.size());
	out.append(reinterpret_cast<const char *>(footer_bytes), sizeof(footer_bytes));

	return out;
}
