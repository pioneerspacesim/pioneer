// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "JsonUtils.h"
#include "../../src/utils.h"
#include "../../src/GameSaveError.h" // Need this for the exceptions

extern "C" {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#include "miniz/miniz.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

namespace {
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
};

#define USE_STRING_VERSIONS

void VectorToJson(Json::Value &jsonObj, const vector3f &vec, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[128];
	Vector3fToStr(vec, str, 128);
	jsonObj[name] = str; // Add vector array to supplied object.
#else
	Json::Value vecArray(Json::arrayValue); // Create JSON array to contain vector data.
	vecArray[0] = FloatToStr(vec.x);
	vecArray[1] = FloatToStr(vec.y);
	vecArray[2] = FloatToStr(vec.z);
	jsonObj[name] = vecArray; // Add vector array to supplied object.
#endif
}

void VectorToJson(Json::Value &jsonObj, const vector3d &vec, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[128];
	Vector3dToStr(vec, str, 128);
	jsonObj[name] = str; // Add vector array to supplied object.
#else
	Json::Value vecArray(Json::arrayValue); // Create JSON array to contain vector data.
	vecArray[0] = DoubleToStr(vec.x);
	vecArray[1] = DoubleToStr(vec.y);
	vecArray[2] = DoubleToStr(vec.z);
	jsonObj[name] = vecArray; // Add vector array to supplied object.
#endif
}

void QuaternionToJson(Json::Value &jsonObj, const Quaternionf &quat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	Json::Value quatArray(Json::arrayValue); // Create JSON array to contain quaternion data.
	quatArray[0] = FloatToStr(quat.w);
	quatArray[1] = FloatToStr(quat.x);
	quatArray[2] = FloatToStr(quat.y);
	quatArray[3] = FloatToStr(quat.z);
	jsonObj[name] = quatArray; // Add quaternion array to supplied object.
}

void QuaternionToJson(Json::Value &jsonObj, const Quaterniond &quat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	Json::Value quatArray(Json::arrayValue); // Create JSON array to contain quaternion data.
	quatArray[0] = DoubleToStr(quat.w);
	quatArray[1] = DoubleToStr(quat.x);
	quatArray[2] = DoubleToStr(quat.y);
	quatArray[3] = DoubleToStr(quat.z);
	jsonObj[name] = quatArray; // Add quaternion array to supplied object.
}

void MatrixToJson(Json::Value &jsonObj, const matrix3x3f &mat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[512];
	Matrix3x3fToStr(mat, str, 512);
	jsonObj[name] = str;
#else
	Json::Value matArray(Json::arrayValue); // Create JSON array to contain matrix data.
	matArray[0] = FloatToStr(mat[0]);
	matArray[1] = FloatToStr(mat[1]);
	matArray[2] = FloatToStr(mat[2]);
	matArray[3] = FloatToStr(mat[3]);
	matArray[4] = FloatToStr(mat[4]);
	matArray[5] = FloatToStr(mat[5]);
	matArray[6] = FloatToStr(mat[6]);
	matArray[7] = FloatToStr(mat[7]);
	matArray[8] = FloatToStr(mat[8]);
	jsonObj[name] = matArray; // Add matrix array to supplied object.
#endif
}

void MatrixToJson(Json::Value &jsonObj, const matrix3x3d &mat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[512];
	Matrix3x3dToStr(mat, str, 512);
	jsonObj[name] = str;
#else
	Json::Value matArray(Json::arrayValue); // Create JSON array to contain matrix data.
	matArray[0] = DoubleToStr(mat[0]);
	matArray[1] = DoubleToStr(mat[1]);
	matArray[2] = DoubleToStr(mat[2]);
	matArray[3] = DoubleToStr(mat[3]);
	matArray[4] = DoubleToStr(mat[4]);
	matArray[5] = DoubleToStr(mat[5]);
	matArray[6] = DoubleToStr(mat[6]);
	matArray[7] = DoubleToStr(mat[7]);
	matArray[8] = DoubleToStr(mat[8]);
	jsonObj[name] = matArray; // Add matrix array to supplied object.
#endif
}

void MatrixToJson(Json::Value &jsonObj, const matrix4x4f &mat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[512];
	Matrix4x4fToStr(mat, str, 512);
	jsonObj[name] = str;
#else
	Json::Value matArray(Json::arrayValue); // Create JSON array to contain matrix data.
	matArray[0] = FloatToStr(mat[0]);
	matArray[1] = FloatToStr(mat[1]);
	matArray[2] = FloatToStr(mat[2]);
	matArray[3] = FloatToStr(mat[3]);
	matArray[4] = FloatToStr(mat[4]);
	matArray[5] = FloatToStr(mat[5]);
	matArray[6] = FloatToStr(mat[6]);
	matArray[7] = FloatToStr(mat[7]);
	matArray[8] = FloatToStr(mat[8]);
	matArray[9] = FloatToStr(mat[9]);
	matArray[10] = FloatToStr(mat[10]);
	matArray[11] = FloatToStr(mat[11]);
	matArray[12] = FloatToStr(mat[12]);
	matArray[13] = FloatToStr(mat[13]);
	matArray[14] = FloatToStr(mat[14]);
	matArray[15] = FloatToStr(mat[15]);
	jsonObj[name] = matArray; // Add matrix array to supplied object.
#endif
}

void MatrixToJson(Json::Value &jsonObj, const matrix4x4d &mat, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	char str[512];
	Matrix4x4dToStr(mat, str, 512);
	jsonObj[name] = str;
#else
	Json::Value matArray(Json::arrayValue); // Create JSON array to contain matrix data.
	matArray[0] = DoubleToStr(mat[0]);
	matArray[1] = DoubleToStr(mat[1]);
	matArray[2] = DoubleToStr(mat[2]);
	matArray[3] = DoubleToStr(mat[3]);
	matArray[4] = DoubleToStr(mat[4]);
	matArray[5] = DoubleToStr(mat[5]);
	matArray[6] = DoubleToStr(mat[6]);
	matArray[7] = DoubleToStr(mat[7]);
	matArray[8] = DoubleToStr(mat[8]);
	matArray[9] = DoubleToStr(mat[9]);
	matArray[10] = DoubleToStr(mat[10]);
	matArray[11] = DoubleToStr(mat[11]);
	matArray[12] = DoubleToStr(mat[12]);
	matArray[13] = DoubleToStr(mat[13]);
	matArray[14] = DoubleToStr(mat[14]);
	matArray[15] = DoubleToStr(mat[15]);
	jsonObj[name] = matArray; // Add matrix array to supplied object.
#endif
}

void ColorToJson(Json::Value &jsonObj, const Color3ub &col, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	Json::Value colArray(Json::arrayValue); // Create JSON array to contain color data.
	colArray[0] = col.r;
	colArray[1] = col.g;
	colArray[2] = col.b;
	jsonObj[name] = colArray; // Add color array to supplied object.
}

void ColorToJson(Json::Value &jsonObj, const Color4ub &col, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	Json::Value colArray(Json::arrayValue); // Create JSON array to contain color data.
	colArray[0] = col.r;
	colArray[1] = col.g;
	colArray[2] = col.b;
	colArray[3] = col.a;
	jsonObj[name] = colArray; // Add color array to supplied object.
}

void BinStrToJson(Json::Value &jsonObj, const std::string &binStr, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	// compress in memory, write to open file
	size_t outSize = 0;
	void *pCompressedData = tdefl_compress_mem_to_heap(binStr.data(), binStr.length(), &outSize, 128);
	assert(pCompressedData); // can we fail to compress?
	if (pCompressedData) {
		Json::Value binStrArray(Json::arrayValue); // Create JSON array to contain binary string data.
		binStrArray.resize(outSize); // Pre-Allocate (packed) space for it
		// Packed everything that fits into our 4-byte packing
		for (uint32_t charIndex = 0; charIndex < outSize; ++charIndex) {
			binStrArray[charIndex] = int(((uint8_t*)pCompressedData)[charIndex]);
		}
		// Add compressed and packed binary string array to supplied object.
		jsonObj[name] = binStrArray;
		// release the compressed data
		mz_free(pCompressedData);
	}
}

void JsonToVector(vector3f *pVec, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value vecStr = jsonObj[name.c_str()];
	if (!vecStr.isString()) throw SavedGameCorruptException();

	StrToVector3f(vecStr.asCString(), *pVec);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value vecArray = jsonObj[name.c_str()];
	if (!vecArray.isArray()) throw SavedGameCorruptException();
	if (vecArray.size() != 3) throw SavedGameCorruptException();

	pVec->x = StrToFloat(vecArray[0].asString());
	pVec->y = StrToFloat(vecArray[1].asString());
	pVec->z = StrToFloat(vecArray[2].asString());
#endif
}

void JsonToVector(vector3d *pVec, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value vecStr = jsonObj[name.c_str()];
	if (!vecStr.isString()) throw SavedGameCorruptException();

	StrToVector3d(vecStr.asCString(), *pVec);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value vecArray = jsonObj[name.c_str()];
	if (!vecArray.isArray()) throw SavedGameCorruptException();
	if (vecArray.size() != 3) throw SavedGameCorruptException();

	pVec->x = StrToDouble(vecArray[0].asString());
	pVec->y = StrToDouble(vecArray[1].asString());
	pVec->z = StrToDouble(vecArray[2].asString());
#endif
}

void JsonToQuaternion(Quaternionf *pQuat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value quatArray = jsonObj[name.c_str()];
	if (!quatArray.isArray()) throw SavedGameCorruptException();
	if (quatArray.size() != 4) throw SavedGameCorruptException();

	pQuat->w = StrToFloat(quatArray[0].asString());
	pQuat->x = StrToFloat(quatArray[1].asString());
	pQuat->y = StrToFloat(quatArray[2].asString());
	pQuat->z = StrToFloat(quatArray[3].asString());
}

void JsonToQuaternion(Quaterniond *pQuat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value quatArray = jsonObj[name.c_str()];
	if (!quatArray.isArray()) throw SavedGameCorruptException();
	if (quatArray.size() != 4) throw SavedGameCorruptException();

	pQuat->w = StrToDouble(quatArray[0].asString());
	pQuat->x = StrToDouble(quatArray[1].asString());
	pQuat->y = StrToDouble(quatArray[2].asString());
	pQuat->z = StrToDouble(quatArray[3].asString());
}

void JsonToMatrix(matrix3x3f *pMat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matStr = jsonObj[name.c_str()];
	if (!matStr.isString()) throw SavedGameCorruptException();
	StrToMatrix3x3f(matStr.asCString(), *pMat);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matArray = jsonObj[name.c_str()];
	if (!matArray.isArray()) throw SavedGameCorruptException();
	if (matArray.size() != 9) throw SavedGameCorruptException();

	(*pMat)[0] = StrToFloat(matArray[0].asString());
	(*pMat)[1] = StrToFloat(matArray[1].asString());
	(*pMat)[2] = StrToFloat(matArray[2].asString());
	(*pMat)[3] = StrToFloat(matArray[3].asString());
	(*pMat)[4] = StrToFloat(matArray[4].asString());
	(*pMat)[5] = StrToFloat(matArray[5].asString());
	(*pMat)[6] = StrToFloat(matArray[6].asString());
	(*pMat)[7] = StrToFloat(matArray[7].asString());
	(*pMat)[8] = StrToFloat(matArray[8].asString());
#endif
}

void JsonToMatrix(matrix3x3d *pMat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matStr = jsonObj[name.c_str()];
	if (!matStr.isString()) throw SavedGameCorruptException();
	StrToMatrix3x3d(matStr.asCString(), *pMat);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matArray = jsonObj[name.c_str()];
	if (!matArray.isArray()) throw SavedGameCorruptException();
	if (matArray.size() != 9) throw SavedGameCorruptException();

	(*pMat)[0] = StrToDouble(matArray[0].asString());
	(*pMat)[1] = StrToDouble(matArray[1].asString());
	(*pMat)[2] = StrToDouble(matArray[2].asString());
	(*pMat)[3] = StrToDouble(matArray[3].asString());
	(*pMat)[4] = StrToDouble(matArray[4].asString());
	(*pMat)[5] = StrToDouble(matArray[5].asString());
	(*pMat)[6] = StrToDouble(matArray[6].asString());
	(*pMat)[7] = StrToDouble(matArray[7].asString());
	(*pMat)[8] = StrToDouble(matArray[8].asString());
#endif
}

void JsonToMatrix(matrix4x4f *pMat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matStr = jsonObj[name.c_str()];
	if (!matStr.isString()) throw SavedGameCorruptException();
	StrToMatrix4x4f(matStr.asCString(), *pMat);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matArray = jsonObj[name.c_str()];
	if (!matArray.isArray()) throw SavedGameCorruptException();
	if (matArray.size() != 16) throw SavedGameCorruptException();

	(*pMat)[0] = StrToFloat(matArray[0].asString());
	(*pMat)[1] = StrToFloat(matArray[1].asString());
	(*pMat)[2] = StrToFloat(matArray[2].asString());
	(*pMat)[3] = StrToFloat(matArray[3].asString());
	(*pMat)[4] = StrToFloat(matArray[4].asString());
	(*pMat)[5] = StrToFloat(matArray[5].asString());
	(*pMat)[6] = StrToFloat(matArray[6].asString());
	(*pMat)[7] = StrToFloat(matArray[7].asString());
	(*pMat)[8] = StrToFloat(matArray[8].asString());
	(*pMat)[9] = StrToFloat(matArray[9].asString());
	(*pMat)[10] = StrToFloat(matArray[10].asString());
	(*pMat)[11] = StrToFloat(matArray[11].asString());
	(*pMat)[12] = StrToFloat(matArray[12].asString());
	(*pMat)[13] = StrToFloat(matArray[13].asString());
	(*pMat)[14] = StrToFloat(matArray[14].asString());
	(*pMat)[15] = StrToFloat(matArray[15].asString());
#endif
}

void JsonToMatrix(matrix4x4d *pMat, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.
#ifdef USE_STRING_VERSIONS
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matStr = jsonObj[name.c_str()];
	if (!matStr.isString()) throw SavedGameCorruptException();
	StrToMatrix4x4d(matStr.asCString(), *pMat);
#else
	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value matArray = jsonObj[name.c_str()];
	if (!matArray.isArray()) throw SavedGameCorruptException();
	if (matArray.size() != 16) throw SavedGameCorruptException();

	(*pMat)[0] = StrToDouble(matArray[0].asString());
	(*pMat)[1] = StrToDouble(matArray[1].asString());
	(*pMat)[2] = StrToDouble(matArray[2].asString());
	(*pMat)[3] = StrToDouble(matArray[3].asString());
	(*pMat)[4] = StrToDouble(matArray[4].asString());
	(*pMat)[5] = StrToDouble(matArray[5].asString());
	(*pMat)[6] = StrToDouble(matArray[6].asString());
	(*pMat)[7] = StrToDouble(matArray[7].asString());
	(*pMat)[8] = StrToDouble(matArray[8].asString());
	(*pMat)[9] = StrToDouble(matArray[9].asString());
	(*pMat)[10] = StrToDouble(matArray[10].asString());
	(*pMat)[11] = StrToDouble(matArray[11].asString());
	(*pMat)[12] = StrToDouble(matArray[12].asString());
	(*pMat)[13] = StrToDouble(matArray[13].asString());
	(*pMat)[14] = StrToDouble(matArray[14].asString());
	(*pMat)[15] = StrToDouble(matArray[15].asString());
#endif
}

void JsonToColor(Color3ub *pCol, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value colArray = jsonObj[name.c_str()];
	if (!colArray.isArray()) throw SavedGameCorruptException();
	if (colArray.size() != 3) throw SavedGameCorruptException();

	pCol->r = colArray[0].asUInt();
	pCol->g = colArray[1].asUInt();
	pCol->b = colArray[2].asUInt();
}

void JsonToColor(Color4ub *pCol, const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value colArray = jsonObj[name.c_str()];
	if (!colArray.isArray()) throw SavedGameCorruptException();
	if (colArray.size() != 4) throw SavedGameCorruptException();

	pCol->r = colArray[0].asUInt();
	pCol->g = colArray[1].asUInt();
	pCol->b = colArray[2].asUInt();
	pCol->a = colArray[3].asUInt();
}

std::string JsonToBinStr(const Json::Value &jsonObj, const std::string &name)
{
	PROFILE_SCOPED()
	assert(!name.empty()); // Can't do anything if no name supplied.

	if (!jsonObj.isMember(name.c_str())) throw SavedGameCorruptException();
	Json::Value binStrArray = jsonObj[name.c_str()];
	if (!binStrArray.isArray()) throw SavedGameCorruptException();

	const uint32_t arraySize = binStrArray.size();
	std::unique_ptr<uint8_t[]> compStr(new uint8_t[arraySize]);
	for (uint32_t charIndex = 0; charIndex < arraySize; ++charIndex) {
		compStr[charIndex] = uint8_t(binStrArray[charIndex].asInt());
	}
	size_t outSize = 0;
	void *pDecompressedData = tinfl_decompress_mem_to_heap(&compStr[0], arraySize, &outSize, 0);
	assert(pDecompressedData);
	std::string binStr;
	if (pDecompressedData) {
		for (size_t charIndex = 0; charIndex < outSize; ++charIndex) {
			binStr += static_cast<char*>(pDecompressedData)[charIndex];
		}
		mz_free(pDecompressedData);
	} else {
		throw SavedGameCorruptException();
	}

	return binStr;
}
