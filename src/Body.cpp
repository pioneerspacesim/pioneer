// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Body.h"
#include "Frame.h"
#include "Star.h"
#include "Planet.h"
#include "CargoBody.h"
#include "SpaceStation.h"
#include "Ship.h"
#include "Player.h"
#include "Projectile.h"
#include "Missile.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Space.h"
#include "Game.h"
#include "LuaEvent.h"
#include "json/JsonUtils.h"

Body::Body() : PropertiedObject(Lua::manager)
	, m_flags(0)
	, m_interpPos(0.0)
	, m_interpOrient(matrix3x3d::Identity())
	, m_pos(0.0)
	, m_orient(matrix3x3d::Identity())
	, m_frame(0)
	, m_dead(false)
	, m_clipRadius(0.0)
	, m_physRadius(0.0)
{
	Properties().Set("label", m_label);
}

Body::~Body()
{
}

void Body::SaveToJson(Json::Value &jsonObj, Space *space)
{
	Json::Value bodyObj(Json::objectValue); // Create JSON object to contain body data.

	Properties().SaveToJson(bodyObj);
	bodyObj["index_for_frame"] = space->GetIndexForFrame(m_frame);
	bodyObj["label"] = m_label;
	bodyObj["dead"] = m_dead;

	VectorToJson(bodyObj, m_pos, "pos");
	MatrixToJson(bodyObj, m_orient, "orient");
	bodyObj["phys_radius"] = DoubleToStr(m_physRadius);
	bodyObj["clip_radius"] = DoubleToStr(m_clipRadius);

	jsonObj["body"] = bodyObj; // Add body object to supplied object.
}

void Body::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	if (!jsonObj.isMember("body")) throw SavedGameCorruptException();
	Json::Value bodyObj = jsonObj["body"];

	if (!bodyObj.isMember("index_for_frame")) throw SavedGameCorruptException();
	if (!bodyObj.isMember("label")) throw SavedGameCorruptException();
	if (!bodyObj.isMember("dead")) throw SavedGameCorruptException();
	if (!bodyObj.isMember("phys_radius")) throw SavedGameCorruptException();
	if (!bodyObj.isMember("clip_radius")) throw SavedGameCorruptException();

	Properties().LoadFromJson(bodyObj);
	m_frame = space->GetFrameByIndex(bodyObj["index_for_frame"].asUInt());
	m_label = bodyObj["label"].asString();
	Properties().Set("label", m_label);
	m_dead = bodyObj["dead"].asBool();

	JsonToVector(&m_pos, bodyObj, "pos");
	JsonToMatrix(&m_orient, bodyObj, "orient");
	m_physRadius = StrToDouble(bodyObj["phys_radius"].asString());
	m_clipRadius = StrToDouble(bodyObj["clip_radius"].asString());
}

void Body::ToJson(Json::Value &jsonObj, Space *space)
{
	jsonObj["body_type"] = int(GetType());

	switch (GetType()) {
	case Object::STAR:
	case Object::PLANET:
	case Object::SPACESTATION:
	case Object::SHIP:
	case Object::PLAYER:
	case Object::MISSILE:
	case Object::CARGOBODY:
	case Object::PROJECTILE:
	case Object::HYPERSPACECLOUD:
		SaveToJson(jsonObj, space);
		break;
	default:
		assert(0);
	}
}

Body *Body::FromJson(const Json::Value &jsonObj, Space *space)
{
	if (!jsonObj.isMember("body_type")) throw SavedGameCorruptException();

	Body *b = 0;
	Object::Type type = Object::Type(jsonObj["body_type"].asInt());
	switch (type) {
	case Object::STAR:
		b = new Star(); break;
	case Object::PLANET:
		b = new Planet(); break;
	case Object::SPACESTATION:
		b = new SpaceStation(); break;
	case Object::SHIP:
		b = new Ship(); break;
	case Object::PLAYER:
		b = new Player(); break;
	case Object::MISSILE:
		b = new Missile(); break;
	case Object::PROJECTILE:
		b = new Projectile(); break;
	case Object::CARGOBODY:
		b = new CargoBody(); break;
	case Object::HYPERSPACECLOUD:
		b = new HyperspaceCloud(); break;
	default:
		assert(0);
	}
	b->LoadFromJson(jsonObj, space);
	return b;
}

vector3d Body::GetPositionRelTo(const Frame *relTo) const
{
	vector3d fpos = m_frame->GetPositionRelTo(relTo);
	matrix3x3d forient = m_frame->GetOrientRelTo(relTo);
	return forient * GetPosition() + fpos;
}

vector3d Body::GetInterpPositionRelTo(const Frame *relTo) const
{
	vector3d fpos = m_frame->GetInterpPositionRelTo(relTo);
	matrix3x3d forient = m_frame->GetInterpOrientRelTo(relTo);
	return forient * GetInterpPosition() + fpos;
}

vector3d Body::GetPositionRelTo(const Body *relTo) const
{
	return GetPositionRelTo(relTo->m_frame) - relTo->GetPosition();
}

vector3d Body::GetInterpPositionRelTo(const Body *relTo) const
{
	return GetInterpPositionRelTo(relTo->m_frame) - relTo->GetInterpPosition();
}

matrix3x3d Body::GetOrientRelTo(const Frame *relTo) const
{
	matrix3x3d forient = m_frame->GetOrientRelTo(relTo);
	return forient * GetOrient();
}

matrix3x3d Body::GetInterpOrientRelTo(const Frame *relTo) const
{
	matrix3x3d forient = m_frame->GetInterpOrientRelTo(relTo);
	return forient * GetInterpOrient();
}

vector3d Body::GetVelocityRelTo(const Frame *relTo) const
{
	matrix3x3d forient = m_frame->GetOrientRelTo(relTo);
	vector3d vel = GetVelocity();
	if (m_frame != relTo) vel -= m_frame->GetStasisVelocity(GetPosition());
	return forient * vel + m_frame->GetVelocityRelTo(relTo);
}

vector3d Body::GetVelocityRelTo(const Body *relTo) const
{
	return GetVelocityRelTo(relTo->m_frame) - relTo->GetVelocityRelTo(relTo->m_frame);
}

void Body::OrientOnSurface(double radius, double latitude, double longitude)
{
	vector3d up = vector3d(cos(latitude)*cos(longitude), sin(latitude)*cos(longitude), sin(longitude));
	SetPosition(radius * up);

	vector3d right = up.Cross(vector3d(0,0,1)).Normalized();
	SetOrient(matrix3x3d::FromVectors(right, up));
}

void Body::SwitchToFrame(Frame *newFrame)
{
	const vector3d vel = GetVelocityRelTo(newFrame);		// do this first because it uses position
	const vector3d fpos = m_frame->GetPositionRelTo(newFrame);
	const matrix3x3d forient = m_frame->GetOrientRelTo(newFrame);
	SetPosition(forient * GetPosition() + fpos);
	SetOrient(forient * GetOrient());
	SetVelocity(vel + newFrame->GetStasisVelocity(GetPosition()));
	SetFrame(newFrame);

	LuaEvent::Queue("onFrameChanged", this);
}

void Body::UpdateFrame()
{
	if (!(m_flags & FLAG_CAN_MOVE_FRAME)) return;	

	// falling out of frames
	if (m_frame->GetRadius() < GetPosition().Length()) {
		Frame *newFrame = GetFrame()->GetParent();
		if (newFrame) { 						// don't fall out of root frame
			Output("%s leaves frame %s\n", GetLabel().c_str(), GetFrame()->GetLabel().c_str());
			SwitchToFrame(newFrame);
			return;
		}
	}

	// entering into frames
	for (Frame* kid : m_frame->GetChildren()) {
		const vector3d pos = GetPositionRelTo(kid);
		if (pos.Length() >= kid->GetRadius()) continue;
		SwitchToFrame(kid);
		Output("%s enters frame %s\n", GetLabel().c_str(), kid->GetLabel().c_str());
		break;
	}
}

vector3d Body::GetTargetIndicatorPosition(const Frame *relTo) const
{
	return GetInterpPositionRelTo(relTo);
}

void Body::SetLabel(const std::string &label)
{
	m_label = label;
	Properties().Set("label", label);
}
