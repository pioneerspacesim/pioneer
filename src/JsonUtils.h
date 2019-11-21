// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _JSON_UTILS_H
#define _JSON_UTILS_H

#include "Color.h"
#include "FrameId.h"
#include "Json.h"
#include "Quaternion.h"
#include "RefCounted.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace FileSystem {
	class FileSource;
	class FileData;
} // namespace FileSystem

namespace JsonUtils {
	// Low-level load JSON from a file descriptor.
	Json LoadJson(RefCountedPtr<FileSystem::FileData> fd);
	// Load a JSON file from a path and a file source.
	Json LoadJsonFile(const std::string &filename, FileSystem::FileSource &source);
	// Load a JSON file from the game's data sources, optionally applying all
	// files with the the name <filename>.patch as Json Merge Patch (RFC 7386) files
	Json LoadJsonDataFile(const std::string &filename, bool with_merge = true);
	// Loads an optionally-gzipped, optionally-CBOR encoded JSON file from the specified source.
	Json LoadJsonSaveFile(const std::string &filename, FileSystem::FileSource &source);
} // namespace JsonUtils

// To-JSON functions. These are called explicitly, and are passed a reference to the object to fill.
void VectorToJson(Json &jsonObj, const vector3f &vec);
void VectorToJson(Json &jsonObj, const vector3d &vec);
void QuaternionToJson(Json &jsonObj, const Quaternionf &quat);
void QuaternionToJson(Json &jsonObj, const Quaterniond &quat);
void MatrixToJson(Json &jsonObj, const matrix3x3f &mat);
void MatrixToJson(Json &jsonObj, const matrix3x3d &mat);
void MatrixToJson(Json &jsonObj, const matrix4x4f &mat);
void MatrixToJson(Json &jsonObj, const matrix4x4d &mat);
void ColorToJson(Json &jsonObj, const Color3ub &col);
void ColorToJson(Json &jsonObj, const Color4ub &col);
void BinStrToJson(Json &jsonObj, const std::string &str);

// Drivers for automatic serialization of custom types. These are implicitly called by assigning to a Json object.
template <typename T>
void to_json(Json &obj, const vector3<T> &vec) { VectorToJson(obj, vec); }
template <typename T>
void to_json(Json &obj, const Quaternion<T> &vec) { QuaternionToJson(obj, vec); }
template <typename T>
void to_json(Json &obj, const matrix3x3<T> &mat) { MatrixToJson(obj, mat); }
template <typename T>
void to_json(Json &obj, const matrix4x4<T> &mat) { MatrixToJson(obj, mat); }
inline void to_json(Json &obj, const Color3ub &col) { ColorToJson(obj, col); }
inline void to_json(Json &obj, const Color4ub &col) { ColorToJson(obj, col); }
inline void to_json(Json &obj, const FrameId &t) { obj = t.id(); }

// Parse JSON functions. These functions will throw Json::type_error if passed an invalid type.
void JsonToVector(vector3f *vec, const Json &jsonObj);
void JsonToVector(vector3d *vec, const Json &jsonObj);
void JsonToQuaternion(Quaternionf *pQuat, const Json &jsonObj);
void JsonToQuaternion(Quaterniond *pQuat, const Json &jsonObj);
void JsonToMatrix(matrix3x3f *pMat, const Json &jsonObj);
void JsonToMatrix(matrix3x3d *pMat, const Json &jsonObj);
void JsonToMatrix(matrix4x4f *pMat, const Json &jsonObj);
void JsonToMatrix(matrix4x4d *pMat, const Json &jsonObj);
void JsonToColor(Color3ub *pCol, const Json &jsonObj);
void JsonToColor(Color4ub *pCol, const Json &jsonObj);
std::string JsonToBinStr(const Json &jsonObj);

template <typename T>
void from_json(const Json &obj, vector3<T> &vec) { JsonToVector(&vec, obj); }
template <typename T>
void from_json(const Json &obj, Quaternion<T> &vec) { JsonToQuaternion(&vec, obj); }
template <typename T>
void from_json(const Json &obj, matrix3x3<T> &vec) { JsonToMatrix(&vec, obj); }
template <typename T>
void from_json(const Json &obj, matrix4x4<T> &vec) { JsonToMatrix(&vec, obj); }
inline void from_json(const Json &obj, Color3ub &col) { JsonToColor(&col, obj); }
inline void from_json(const Json &obj, Color4ub &col) { JsonToColor(&col, obj); }
inline void from_json(const Json &obj, FrameId &id) { id = (int)obj; }

#endif /* _JSON_UTILS_H */
