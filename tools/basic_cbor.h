// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

/*
	The basic guts of a CBOR encoder - we're not concerned with full standards
	compliance, merely with efficiently outputting valid data that
	nlohmann::json can then read in as JSON data.

	REMEMBER THAT CBOR IS BIG-ENDIAN ENCODED!
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

enum class CBORTag {
	Integer = 0,
	NegInteger = 1,
	ByteString = 2,
	String = 3,
	Array = 4,
	Object = 5,
	Simple = 7
};

namespace cbor {

	// Thanks to https://stackoverflow.com/questions/2782725/converting-float-values-from-big-endian-to-little-endian
	void swap4(uint8_t *data)
	{
		auto *ptr = reinterpret_cast<uint32_t *>(data);
		uint32_t n = *ptr;
		n = ((n >> 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00); // 1234 -> 2143
		n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000); // 2143 -> 4321
		*ptr = n;
	}

	template <typename T>
	void push4(std::vector<uint8_t> &out, const T val)
	{
		size_t cur_size = out.size();
		out.resize(out.size() + 4);
		*reinterpret_cast<T *>(out.data() + cur_size) = val;
		swap4(out.data() + cur_size);
	}

	// Push a tag and an additional information value up to 32 bits.
	void push_tag(std::vector<uint8_t> &out, const CBORTag tag, const uint32_t val)
	{
		uint8_t tagVal = static_cast<uint8_t>(tag) << 5;
		if (val < 24)
			out.push_back(tagVal | (val & 0xFF));

		else if (val < 1 << 8) {
			out.push_back(tagVal | 24);
			out.push_back(val & 0xFF);
		}

		else if (val < 1 << 16) {
			out.push_back(tagVal | 25);
			out.push_back(val >> 8 & 0xFF);
			out.push_back(val & 0xFF);
		}

		else {
			out.push_back(tagVal | 26);
			push4(out, val);
		}
	}

	void push_simple(std::vector<uint8_t> &out, const uint8_t simple)
	{
		out.push_back(static_cast<uint8_t>(CBORTag::Simple) << 5 | (simple & 0x1F));
		if (simple > 31)
			out.push_back(simple);
	}

	void push_float(std::vector<uint8_t> &out, const float val)
	{
		push_simple(out, 26);
		push4(out, val);
	}

	void push_string(std::vector<uint8_t> &out, const std::string &string)
	{
		push_tag(out, CBORTag::String, string.size());
		if (string.size() > 0) {
			size_t cur_size = out.size();
			out.resize(out.size() + string.size());

			std::memcpy(out.data() + cur_size, string.data(), string.size());
		}
	}

	void push_int(std::vector<uint8_t> &out, const int val)
	{
		if (val < 0)
			push_tag(out, CBORTag::NegInteger, -1 - val);
		else
			push_tag(out, CBORTag::Integer, val);
	}
} // namespace cbor
