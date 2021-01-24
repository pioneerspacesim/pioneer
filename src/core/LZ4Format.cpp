// Copyright © 2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LZ4Format.h"
#include "lz4/lz4frame.h"
#include "profiler/Profiler.h"
#include <SDL_endian.h>
#include <functional>
#include <memory>

bool lz4::IsLZ4Format(const char *data, size_t length)
{
	const uint32_t magic = *reinterpret_cast<const uint32_t *>(data);

	return magic == SDL_SwapLE32(0x184D2204);
}

template <typename T>
static void checkError(std::size_t errorCode)
{
	if (LZ4F_isError(errorCode)) {
		throw T(LZ4F_getErrorName(errorCode));
	}
}

std::string lz4::DecompressLZ4(const std::string_view data)
{
	PROFILE_SCOPED()
	LZ4F_dctx *_tmp;
	LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&_tmp, LZ4F_VERSION);
	checkError<lz4::DecompressionFailedException>(err);

	std::unique_ptr<LZ4F_dctx, std::function<size_t(LZ4F_dctx *)>> dctx(_tmp, LZ4F_freeDecompressionContext);

	const char *read_ptr = data.data();
	std::size_t read_len = data.size();
	std::size_t write_len = 0;
	const char *const end_ptr = read_ptr + read_len;

	LZ4F_frameInfo_t frame = LZ4F_INIT_FRAMEINFO;
	// get the frame info: resets read_len to the number of bytes consumed,
	// and fills nextLen with the number of bytes it expects to read.
	std::size_t nextLen = LZ4F_getFrameInfo(dctx.get(), &frame, read_ptr, &read_len);
	checkError<lz4::DecompressionFailedException>(nextLen);

	const std::size_t buffer_len = 1 << 16;
	std::unique_ptr<char[]> decompress_buffer(new char[buffer_len]);

	std::string out;

	while (nextLen != 0) {
		// advance the read pointer by the number of bytes consumed last time
		read_ptr += read_len;
		// and initialize read_len with the number of available bytes
		read_len = end_ptr - read_ptr;
		write_len = buffer_len;

		// read_len is set to the number of bytes consumed
		nextLen = LZ4F_decompress(dctx.get(), decompress_buffer.get(), &write_len, read_ptr, &read_len, NULL);
		checkError<lz4::DecompressionFailedException>(nextLen);

		out.append(decompress_buffer.get(), write_len);
	}

	decompress_buffer.reset();
	dctx.reset();

	return out;
}

std::string lz4::CompressLZ4(const std::string_view data, const int lz4_preset)
{
	PROFILE_SCOPED()
	LZ4F_preferences_t pref = LZ4F_INIT_PREFERENCES;
	pref.compressionLevel = lz4_preset;

	std::size_t compressBound = LZ4F_compressFrameBound(data.size(), &pref);
	// null-initialize the string to the specified length
	std::unique_ptr<char[]> out(new char[compressBound]);

	std::size_t outSize = LZ4F_compressFrame(out.get(), compressBound, data.data(), data.size(), &pref);
	checkError<lz4::CompressionFailedException>(outSize);

	return std::string(out.get(), outSize);
}
