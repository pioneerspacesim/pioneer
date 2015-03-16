// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _JSON_UTILS_H
#define _JSON_UTILS_H

#include "json/json.h"
#include "../../src/vector3.h"
#include "../../src/Quaternion.h"
#include "../../src/matrix3x3.h"
#include "../../src/matrix4x4.h"
#include "../../src/Color.h"

// To-JSON functions.
void VectorToJson(Json::Value &jsonObj, const vector3f &vec, const std::string &name);
void VectorToJson(Json::Value &jsonObj, const vector3d &vec, const std::string &name);
void QuaternionToJson(Json::Value &jsonObj, const Quaternionf &quat, const std::string &name);
void QuaternionToJson(Json::Value &jsonObj, const Quaterniond &quat, const std::string &name);
void MatrixToJson(Json::Value &jsonObj, const matrix3x3f &mat, const std::string &name);
void MatrixToJson(Json::Value &jsonObj, const matrix3x3d &mat, const std::string &name);
void MatrixToJson(Json::Value &jsonObj, const matrix4x4f &mat, const std::string &name);
void MatrixToJson(Json::Value &jsonObj, const matrix4x4d &mat, const std::string &name);
void ColorToJson(Json::Value &jsonObj, const Color3ub &col, const std::string &name);
void ColorToJson(Json::Value &jsonObj, const Color4ub &col, const std::string &name);
void BinStrToJson(Json::Value &jsonObj, const std::string &str, const std::string &name);

// Parse JSON functions.
void JsonToVector(vector3f *pVec, const Json::Value &jsonObj, const std::string &name);
void JsonToVector(vector3d *pVec, const Json::Value &jsonObj, const std::string &name);
void JsonToQuaternion(Quaternionf *pQuat, const Json::Value &jsonObj, const std::string &name);
void JsonToQuaternion(Quaterniond *pQuat, const Json::Value &jsonObj, const std::string &name);
void JsonToMatrix(matrix3x3f *pMat, const Json::Value &jsonObj, const std::string &name);
void JsonToMatrix(matrix3x3d *pMat, const Json::Value &jsonObj, const std::string &name);
void JsonToMatrix(matrix4x4f *pMat, const Json::Value &jsonObj, const std::string &name);
void JsonToMatrix(matrix4x4d *pMat, const Json::Value &jsonObj, const std::string &name);
void JsonToColor(Color3ub *pCol, const Json::Value &jsonObj, const std::string &name);
void JsonToColor(Color4ub *pCol, const Json::Value &jsonObj, const std::string &name);
std::string JsonToBinStr(const Json::Value &jsonObj, const std::string &name);

#endif /* _JSON_UTILS_H */
