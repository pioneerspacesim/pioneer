// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "JsonUtils.h"
#include "FileSystem.h"
#include "base64/base64.hpp"
#include "core/GZipFormat.h"
#include "utils.h"
#include <cmath>

extern "C" {
#include "miniz/miniz.h"
}

namespace {
	// XXX currently unused, uncomment if/when needed again.
	// also has some endianness issues,
	/*
	uint32_t pack4char(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d)
	{
		return ((a << 24) | (b << 16) | (c << 8) | d);
	}

	void unpack4char(const uint32_t packed, uint8_t &a, uint8_t &b, uint8_t &c, uint8_t &d)
	{
		a = ((packed >> 24) & 0xff);
		b = ((packed >> 16) & 0xff);
		c = ((packed >> 8) & 0xff);
		d = (packed & 0xff);
	}
*/

	static const vector3f zeroVector3f(0.0f);
	static const vector3d zeroVector3d(0.0);
	static const Quaternionf identityQuaternionf(1.0f, 0.0f, 0.0f, 0.0f);
	static const Quaterniond identityQuaterniond(1.0, 0.0, 0.0, 0.0);
} // namespace

namespace JsonUtils {
	Json LoadJson(RefCountedPtr<FileSystem::FileData> fd)
	{
		if (!fd) return Json();

		Json out;

		try {
			out = Json::parse(std::string(fd->GetData(), fd->GetSize()));
		} catch (Json::parse_error &e) {
			Output("error in JSON file '%s': %s\n", fd->GetInfo().GetPath().c_str(), e.what());
			return nullptr;
		}

		return out;
	}

	Json LoadJsonFile(const std::string &filename, FileSystem::FileSource &source)
	{
		return LoadJson(source.ReadFile(filename));
	}

	Json LoadJsonDataFile(const std::string &filename, bool with_merge)
	{
		Json out = LoadJsonFile(filename, FileSystem::gameDataFiles);
		if (out.is_null() || !with_merge) return out;

		for (auto info : FileSystem::gameDataFiles.LookupAll(filename + ".patch")) {
			Json patch = LoadJson(info.Read());
			if (!patch.is_null()) out.merge_patch(patch);
		}

		return out;
	}

	Json LoadJsonSaveFile(const std::string &filename, FileSystem::FileSource &source)
	{
		auto file = source.ReadFile(filename);
		if (!file) return nullptr;
		const auto file_data = std::string(file->GetData(), file->GetSize());
		const unsigned char *dataPtr = reinterpret_cast<const unsigned char *>(&file_data[0]);
		try {
			std::string plain_data;
			if (gzip::IsGZipFormat(dataPtr, file_data.size())) {
				plain_data = gzip::DecompressDeflateOrGZip(dataPtr, file_data.size());
			} else {
				plain_data = file_data;
			}

			Json rootNode;
			try {
				// Allow loading files in JSON format as well as CBOR
				if (plain_data[0] == '{')
					return Json::parse(plain_data);
				else
					return Json::from_cbor(plain_data);
			} catch (Json::parse_error &e) {
				Output("error in JSON file '%s': %s\n", file->GetInfo().GetPath().c_str(), e.what());
				return nullptr;
			}
		} catch (gzip::DecompressionFailedException) {
			return nullptr;
		}
	}
} // namespace JsonUtils

#define USE_STRING_VERSIONS

void VectorToJson(Json &jsonObj, const vector2f &vec)
{
	// Create JSON array to contain vector data.
	jsonObj = Json::array({ vec.x, vec.y });
}

void VectorToJson(Json &jsonObj, const vector2d &vec)
{
	// Create JSON array to contain vector data.
	jsonObj = Json::array({ vec.x, vec.y });
}

void VectorToJson(Json &jsonObj, const vector3f &vec)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (vec == zeroVector3f)
		return; // don't store zero vector

	char str[128];
	Vector3fToStr(vec, str, 128);
	jsonObj = str; // Add vector array to supplied object.
#else
	// Create JSON array to contain vector data.
	jsonObj = Json::array({ vec.x, vec.y, vec.z });
#endif
}

void VectorToJson(Json &jsonObj, const vector3d &vec)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (vec == zeroVector3d)
		return; // don't store zero vector
	char str[128];
	Vector3dToStr(vec, str, 128);
	jsonObj = str; // Add vector array to supplied object.
#else
	// Create JSON array to contain vector data.
	jsonObj = Json::array({ vec.x, vec.y, vec.z });
#endif
}

void QuaternionToJson(Json &jsonObj, const Quaternionf &quat)
{
	PROFILE_SCOPED()

	if (memcmp(&quat, &identityQuaternionf, sizeof(Quaternionf)) == 0)
		return;
	jsonObj = Json::array({ quat.w, quat.x, quat.y, quat.z });
}

void QuaternionToJson(Json &jsonObj, const Quaterniond &quat)
{
	PROFILE_SCOPED()
	if (memcmp(&quat, &identityQuaterniond, sizeof(Quaterniond)) == 0)
		return;

	jsonObj = Json::array({ quat.w, quat.x, quat.y, quat.z });
}

void MatrixToJson(Json &jsonObj, const matrix3x3f &mat)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!memcmp(&matrix3x3fIdentity, &mat, sizeof(matrix3x3f))) return;
	char str[512];
	Matrix3x3fToStr(mat, str, 512);
	jsonObj = str;
#else
	jsonObj = Json::array({ mat[0], mat[1], mat[2],
		mat[3], mat[4], mat[5],
		mat[6], mat[7], mat[8] });
#endif
}

void MatrixToJson(Json &jsonObj, const matrix3x3d &mat)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!memcmp(&matrix3x3dIdentity, &mat, sizeof(matrix3x3d))) return;
	char str[512];
	Matrix3x3dToStr(mat, str, 512);
	jsonObj = str;
#else
	jsonObj = Json::array({ mat[0], mat[1], mat[2],
		mat[3], mat[4], mat[5],
		mat[6], mat[7], mat[8] });
#endif
}

void MatrixToJson(Json &jsonObj, const matrix4x4f &mat)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!memcmp(&matrix4x4fIdentity, &mat, sizeof(matrix4x4f))) return;
	char str[512];
	Matrix4x4fToStr(mat, str, 512);
	jsonObj = str;
#else
	jsonObj = Json::array({
		mat[0],
		mat[1],
		mat[2],
		mat[3],
		mat[4],
		mat[5],
		mat[6],
		mat[7],
		mat[8],
		mat[9],
		mat[10],
		mat[11],
		mat[12],
		mat[13],
		mat[14],
		mat[15],
	});
#endif
}

void MatrixToJson(Json &jsonObj, const matrix4x4d &mat)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!memcmp(&matrix4x4dIdentity, &mat, sizeof(matrix4x4d))) return;
	char str[512];
	Matrix4x4dToStr(mat, str, 512);
	jsonObj = str;
#else
	jsonObj = Json::array({
		mat[0],
		mat[1],
		mat[2],
		mat[3],
		mat[4],
		mat[5],
		mat[6],
		mat[7],
		mat[8],
		mat[9],
		mat[10],
		mat[11],
		mat[12],
		mat[13],
		mat[14],
		mat[15],
	});
#endif
}

void ColorToJson(Json &jsonObj, const Color3ub &col)
{
	PROFILE_SCOPED()

	jsonObj[0] = col.r;
	jsonObj[1] = col.g;
	jsonObj[2] = col.b;
}

void ColorToJson(Json &jsonObj, const Color4ub &col)
{
	PROFILE_SCOPED()

	jsonObj[0] = col.r;
	jsonObj[1] = col.g;
	jsonObj[2] = col.b;
	jsonObj[3] = col.a;
}

void BinStrToJson(Json &jsonObj, const std::string &binStr)
{
	PROFILE_SCOPED()

	// compress in memory, write to open file
	size_t outSize = 0;
	void *pCompressedData = tdefl_compress_mem_to_heap(binStr.data(), binStr.length(), &outSize, 128);
	assert(pCompressedData); // can we fail to compress?
	if (pCompressedData) {
		// We encode as base64 to avoid generating invalid UTF-8 data, which breaks the JSON standard.
		// Prealloc a string for the encoded data.
		std::string encodedData = std::string(Base64::EncodedLength(outSize), '\0');
		// Use C++11's contiguous std::string implementation to great effect.
		if (Base64::Encode(static_cast<const char *>(pCompressedData), outSize, &encodedData[0], encodedData.size())) {
			// Store everything in a string.
			jsonObj = std::move(encodedData);
		}
		// release the compressed data
		mz_free(pCompressedData);
	}
}

void JsonToVector(vector2f *pVec, const Json &jsonObj)
{
	pVec->x = jsonObj[0];
	pVec->y = jsonObj[1];
}

void JsonToVector(vector2d *pVec, const Json &jsonObj)
{
	pVec->x = jsonObj[0];
	pVec->y = jsonObj[1];
}

void JsonToVector(vector3f *pVec, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pVec = vector3f(0.0f);
		return;
	}
	std::string vecStr = jsonObj;
	StrToVector3f(vecStr.c_str(), *pVec);
#else
	pVec->x = jsonObj[0];
	pVec->y = jsonObj[1];
	pVec->z = jsonObj[2];
#endif
}

void JsonToVector(vector3d *pVec, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pVec = vector3d(0.0);
		return;
	}
	std::string vecStr = jsonObj;
	StrToVector3d(vecStr.c_str(), *pVec);
#else
	pVec->x = jsonObj[0];
	pVec->y = jsonObj[1];
	pVec->z = jsonObj[2];
#endif
}

void JsonToQuaternion(Quaternionf *pQuat, const Json &jsonObj)
{
	PROFILE_SCOPED()
	if (!jsonObj.is_array()) {
		*pQuat = identityQuaternionf;
		return;
	}

	pQuat->w = jsonObj[0];
	pQuat->x = jsonObj[1];
	pQuat->y = jsonObj[2];
	pQuat->z = jsonObj[3];
}

void JsonToQuaternion(Quaterniond *pQuat, const Json &jsonObj)
{
	PROFILE_SCOPED()
	if (!jsonObj.is_array()) {
		*pQuat = identityQuaterniond;
		return;
	}

	pQuat->w = jsonObj[0];
	pQuat->x = jsonObj[1];
	pQuat->y = jsonObj[2];
	pQuat->z = jsonObj[3];
}

void JsonToMatrix(matrix3x3f *pMat, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pMat = matrix3x3fIdentity;
		return;
	}
	std::string matStr = jsonObj;
	StrToMatrix3x3f(matStr.c_str(), *pMat);
#else
	(*pMat)[0] = jsonObj[0];
	(*pMat)[1] = jsonObj[1];
	(*pMat)[2] = jsonObj[2];
	(*pMat)[3] = jsonObj[3];
	(*pMat)[4] = jsonObj[4];
	(*pMat)[5] = jsonObj[5];
	(*pMat)[6] = jsonObj[6];
	(*pMat)[7] = jsonObj[7];
	(*pMat)[8] = jsonObj[8];
#endif
}

void JsonToMatrix(matrix3x3d *pMat, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pMat = matrix3x3dIdentity;
		return;
	}
	std::string matStr = jsonObj;
	StrToMatrix3x3d(matStr.c_str(), *pMat);
#else
	(*pMat)[0] = jsonObj[0];
	(*pMat)[1] = jsonObj[1];
	(*pMat)[2] = jsonObj[2];
	(*pMat)[3] = jsonObj[3];
	(*pMat)[4] = jsonObj[4];
	(*pMat)[5] = jsonObj[5];
	(*pMat)[6] = jsonObj[6];
	(*pMat)[7] = jsonObj[7];
	(*pMat)[8] = jsonObj[8];
#endif
}

void JsonToMatrix(matrix4x4f *pMat, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pMat = matrix4x4fIdentity;
		return;
	}
	std::string matStr = jsonObj;
	StrToMatrix4x4f(matStr.c_str(), *pMat);
#else
	(*pMat)[0] = jsonObj[0];
	(*pMat)[1] = jsonObj[1];
	(*pMat)[2] = jsonObj[2];
	(*pMat)[3] = jsonObj[3];
	(*pMat)[4] = jsonObj[4];
	(*pMat)[5] = jsonObj[5];
	(*pMat)[6] = jsonObj[6];
	(*pMat)[7] = jsonObj[7];
	(*pMat)[8] = jsonObj[8];
	(*pMat)[9] = jsonObj[9];
	(*pMat)[10] = jsonObj[10];
	(*pMat)[11] = jsonObj[11];
	(*pMat)[12] = jsonObj[12];
	(*pMat)[13] = jsonObj[13];
	(*pMat)[14] = jsonObj[14];
	(*pMat)[15] = jsonObj[15];
#endif
}

void JsonToMatrix(matrix4x4d *pMat, const Json &jsonObj)
{
	PROFILE_SCOPED()
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.is_string()) {
		*pMat = matrix4x4dIdentity;
		return;
	}
	std::string matStr = jsonObj;
	StrToMatrix4x4d(matStr.c_str(), *pMat);
#else
	(*pMat)[0] = jsonObj[0];
	(*pMat)[1] = jsonObj[1];
	(*pMat)[2] = jsonObj[2];
	(*pMat)[3] = jsonObj[3];
	(*pMat)[4] = jsonObj[4];
	(*pMat)[5] = jsonObj[5];
	(*pMat)[6] = jsonObj[6];
	(*pMat)[7] = jsonObj[7];
	(*pMat)[8] = jsonObj[8];
	(*pMat)[9] = jsonObj[9];
	(*pMat)[10] = jsonObj[10];
	(*pMat)[11] = jsonObj[11];
	(*pMat)[12] = jsonObj[12];
	(*pMat)[13] = jsonObj[13];
	(*pMat)[14] = jsonObj[14];
	(*pMat)[15] = jsonObj[15];
#endif
}

void JsonToColor(Color3ub *pCol, const Json &jsonObj)
{
	PROFILE_SCOPED()

	pCol->r = jsonObj[0];
	pCol->g = jsonObj[1];
	pCol->b = jsonObj[2];
}

void JsonToColor(Color4ub *pCol, const Json &jsonObj)
{
	PROFILE_SCOPED()

	pCol->r = jsonObj[0];
	pCol->g = jsonObj[1];
	pCol->b = jsonObj[2];
	pCol->a = jsonObj[3];
}

std::string JsonToBinStr(const Json &jsonObj)
{
	PROFILE_SCOPED()

	// Decode the base64 string into raw binary data.
	std::string binStr;
	if (!Base64::Decode(jsonObj, &binStr)) return binStr;

	size_t outSize = 0;
	void *pDecompressedData = tinfl_decompress_mem_to_heap(binStr.c_str(), binStr.size(), &outSize, 0);
	binStr.clear();
	assert(pDecompressedData);
	if (pDecompressedData) {
		binStr = std::string((char *)pDecompressedData, outSize);
		mz_free(pDecompressedData);
	}

	return binStr;
}

void to_json(Json &obj, const fixed &f)
{
	// produces e.g. "f53/100"
	obj = std::string("f") + std::to_string((f.v & ~f.MASK) >> f.FRAC) + "/" + std::to_string(f.v & f.MASK);
}

void from_json(const Json &obj, fixed &f)
{
	if (obj.is_number_integer())
		f = fixed(obj.get<int64_t>(), 1);

	else if (obj.is_number_float())
		f = fixed::FromDouble(obj.get<double>());

	else {
		std::string str = obj;
		// must have at least f1/1, though can be f1234567/135758548 etc.
		if (str.size() < 4 || obj[0] != 'f')
			throw Json::type_error::create(320, "cannot pickle string to fixed point number");

		char *next_str = const_cast<char *>(str.c_str()) + 1;
		int64_t numerator = std::strtol(next_str, &next_str, 10);

		// handle cases: f/34, f1356, f14+4
		if (numerator == 0 || next_str == nullptr || size_t(next_str - str.c_str()) >= str.size() || *next_str++ != '/')
			throw Json::type_error::create(320, "cannot pickle string to fixed point number");

		int64_t denominator = std::strtol(next_str, &next_str, 10);
		// handle cases f1345/7684gfrty; fixed numbers should not have any garbage data involved
		if (next_str != str.c_str() + str.size())
			throw Json::type_error::create(320, "cannot pickle string to fixed point number");

		f = fixed(numerator, denominator);
	}
}
