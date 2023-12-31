// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Body.h"

#include "BodyComponent.h"
#include "CargoBody.h"
#include "Frame.h"
#include "GameSaveError.h"
#include "HyperspaceCloud.h"
#include "JsonUtils.h"
#include "Missile.h"
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include "Ship.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Star.h"
#include "core/Log.h"
#include "lua/LuaEvent.h"

Body::Body() :
	PropertiedObject(),
	m_interpPos(0.0),
	m_interpOrient(matrix3x3d::Identity()),
	m_pos(0.0),
	m_orient(matrix3x3d::Identity()),
	m_frame(FrameId::Invalid),
	m_dead(false),
	m_clipRadius(0.0),
	m_physRadius(0.0)
{
	Properties().Set("label", m_label);
}

Body::Body(const Json &jsonObj, Space *space) :
	PropertiedObject(),
	m_interpPos(0.0),
	m_interpOrient(matrix3x3d::Identity()),
	m_frame(FrameId::Invalid)
{
	try {
		Json bodyObj = jsonObj["body"];

		Properties().LoadFromJson(bodyObj["properties"]);
		m_frame = bodyObj["index_for_frame"];
		m_label = bodyObj["label"].get<std::string>();
		Properties().Set("label", m_label);
		m_dead = bodyObj["dead"];

		m_pos = bodyObj["pos"];
		m_orient = bodyObj["orient"];
		m_physRadius = bodyObj["phys_radius"];
		m_clipRadius = bodyObj["clip_radius"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

Body::~Body()
{
	size_t idx = 0;
	while (m_components) {
		// get the bit index for each active component and delete it.
		while (!(m_components & 1)) {
			m_components >>= 1;
			idx++;
		}
		BodyComponentDB::GetComponentType(idx)->deleteComponent(this);
		m_components >>= 1;
		idx++;
	}
}

void Body::SaveToJson(Json &jsonObj, Space *space)
{
	PROFILE_SCOPED()

	Json bodyObj = Json::object(); // Create JSON object to contain body data.

	Properties().SaveToJson(bodyObj["properties"]);
	bodyObj["index_for_frame"] = m_frame.id();
	bodyObj["label"] = m_label;
	bodyObj["dead"] = m_dead;

	bodyObj["pos"] = m_pos;
	bodyObj["orient"] = m_orient;
	bodyObj["phys_radius"] = m_physRadius;
	bodyObj["clip_radius"] = m_clipRadius;

	jsonObj["body"] = bodyObj; // Add body object to supplied object.

	Json componentsObj = Json::object();

	// Iterate components and serialize
	size_t components = GetComponentList();
	for (uint32_t index = 0; components && index < 64; (components >>= 1, index++)) {
		if ((components & 1UL) == 0)
			continue;

		auto type = BodyComponentDB::GetComponentType(index);
		BodyComponentDB::SerializerBase *serializer = type->serializer;
		if (serializer) {
			Json serializedComponent = Json::object();
			serializer->toJson(this, serializedComponent, space);

			componentsObj[type->typeName] = serializedComponent;
		}
	}

	jsonObj["components"] = componentsObj;
}

void Body::ToJson(Json &jsonObj, Space *space)
{
	jsonObj["body_type"] = int(GetType());

	switch (GetType()) {
	case ObjectType::STAR:
	case ObjectType::PLANET:
	case ObjectType::SPACESTATION:
	case ObjectType::SHIP:
	case ObjectType::PLAYER:
	case ObjectType::MISSILE:
	case ObjectType::CARGOBODY:
	case ObjectType::PROJECTILE:
	case ObjectType::HYPERSPACECLOUD:
		SaveToJson(jsonObj, space);
		break;
	default:
		assert(0);
	}
}

Body *Body::FromJson(const Json &jsonObj, Space *space)
{
	PROFILE_SCOPED()

	if (!jsonObj["body_type"].is_number_integer())
		throw SavedGameCorruptException();

	Body *body = nullptr;

	ObjectType type = ObjectType(jsonObj["body_type"]);
	switch (type) {
	case ObjectType::STAR:
		body = new Star(jsonObj, space);
		break;
	case ObjectType::PLANET:
		body = new Planet(jsonObj, space);
		break;
	case ObjectType::SPACESTATION:
		body = new SpaceStation(jsonObj, space);
		break;
	case ObjectType::SHIP: {
		Ship *s = new Ship(jsonObj, space);
		// Here because of comments in Ship.cpp on following function
		s->UpdateLuaStats();
		body = static_cast<Body *>(s);
		break;
	}
	case ObjectType::PLAYER: {
		Player *p = new Player(jsonObj, space);
		// Read comments in Ship.cpp on following function
		p->UpdateLuaStats();
		body = static_cast<Body *>(p);
		break;
	}
	case ObjectType::MISSILE:
		body = new Missile(jsonObj, space);
		break;
	case ObjectType::PROJECTILE:
		body = new Projectile(jsonObj, space);
		break;
	case ObjectType::CARGOBODY:
		body = new CargoBody(jsonObj, space);
		break;
	case ObjectType::HYPERSPACECLOUD:
		body = new HyperspaceCloud(jsonObj, space);
		break;
	default:
		assert(0);
	}

	// Iterate component records and deserialize
	const Json &components = jsonObj["components"];
	if (components.is_object()) {
		for (auto pair : components.items()) {
			BodyComponentDB::SerializerBase *serializer = BodyComponentDB::GetSerializer(pair.key());
			if (!serializer) {
				Log::Warning("Cannot deserialize body component '{}'.\n", pair.key());
				continue;
			}

			serializer->fromJson(body, pair.value(), space);
		}
	}

	return body;
}

vector3d Body::GetPositionRelTo(FrameId relToId) const
{
	Frame *frame = Frame::GetFrame(m_frame);

	vector3d fpos = frame->GetPositionRelTo(relToId);
	matrix3x3d forient = frame->GetOrientRelTo(relToId);
	return forient * GetPosition() + fpos;
}

vector3d Body::GetInterpPositionRelTo(FrameId relToId) const
{
	Frame *frame = Frame::GetFrame(m_frame);

	vector3d fpos = frame->GetInterpPositionRelTo(relToId);
	matrix3x3d forient = frame->GetInterpOrientRelTo(relToId);
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

matrix3x3d Body::GetOrientRelTo(FrameId relToId) const
{
	Frame *frame = Frame::GetFrame(m_frame);

	matrix3x3d forient = frame->GetOrientRelTo(relToId);
	return forient * GetOrient();
}

matrix3x3d Body::GetInterpOrientRelTo(FrameId relToId) const
{
	Frame *frame = Frame::GetFrame(m_frame);

	matrix3x3d forient = frame->GetInterpOrientRelTo(relToId);
	return forient * GetInterpOrient();
}

vector3d Body::GetVelocityRelTo(FrameId relToId) const
{
	Frame *frame = Frame::GetFrame(m_frame);

	matrix3x3d forient = frame->GetOrientRelTo(relToId);
	vector3d vel = GetVelocity();
	if (m_frame != relToId) vel -= frame->GetStasisVelocity(GetPosition());
	return forient * vel + frame->GetVelocityRelTo(relToId);
}

vector3d Body::GetVelocityRelTo(const Body *relTo) const
{
	return GetVelocityRelTo(relTo->m_frame) - relTo->GetVelocityRelTo(relTo->m_frame);
}

double Body::GetAltitudeRelTo(const Body *relTo, AltitudeType altType)
{
	if (!relTo) {
		return 0.0;
	}
	vector3d pos = GetPositionRelTo(relTo);
	double center_dist = pos.Length();
	if (relTo->IsType(ObjectType::TERRAINBODY)) {
		const TerrainBody *terrain = static_cast<const TerrainBody *>(relTo);
		vector3d surface_pos = pos.Normalized();
		double radius;
		if (altType != AltitudeType::DEFAULT) {
			radius = altType == AltitudeType::SEA_LEVEL ? terrain->GetSystemBody()->GetRadius() :
														  terrain->GetTerrainHeight(surface_pos);
		} else {
			radius = terrain->GetSystemBody()->GetRadius();
			if (center_dist <= 3.0 * terrain->GetMaxFeatureRadius()) {
				radius = terrain->GetTerrainHeight(surface_pos);
			}
		}
		double altitude = center_dist - radius;
		if (altitude < 0)
			altitude = 0;
		return altitude;
	} else {
		return center_dist;
	}
}

void Body::OrientOnSurface(double radius, double latitude, double longitude)
{
	vector3d up = vector3d(cos(latitude) * cos(longitude), sin(latitude) * cos(longitude), sin(longitude));
	SetPosition(radius * up);

	vector3d right = up.Cross(vector3d(0, 0, 1)).Normalized();
	SetOrient(matrix3x3d::FromVectors(right, up));
}

void Body::SwitchToFrame(FrameId newFrameId)
{
	const Frame *newFrame = Frame::GetFrame(newFrameId);
	const Frame *frame = Frame::GetFrame(m_frame);

	const vector3d vel = GetVelocityRelTo(newFrameId); // do this first because it uses position
	const vector3d fpos = frame->GetPositionRelTo(newFrameId);
	const matrix3x3d forient = frame->GetOrientRelTo(newFrameId);
	SetPosition(forient * GetPosition() + fpos);
	SetOrient(forient * GetOrient());
	SetVelocity(vel + newFrame->GetStasisVelocity(GetPosition()));
	SetFrame(newFrameId);

	LuaEvent::Queue("onFrameChanged", this);
}

void Body::UpdateFrame()
{
	if (!(m_flags & FLAG_CAN_MOVE_FRAME)) return;

	const Frame *frame = Frame::GetFrame(m_frame);

	// falling out of frames
	if (frame->GetRadius() < GetPosition().Length()) {
		FrameId parent = frame->GetParent();
		Frame *newFrame = Frame::GetFrame(parent);
		if (newFrame) { // don't fall out of root frame
			Log::Verbose("{} leaves frame{}\n", GetLabel(), frame->GetLabel());
			SwitchToFrame(parent);
			return;
		}
	}

	// entering into frames
	for (FrameId kid : frame->GetChildren()) {
		Frame *kid_frame = Frame::GetFrame(kid);
		const vector3d pos = GetPositionRelTo(kid);
		if (pos.Length() >= kid_frame->GetRadius()) continue;
		SwitchToFrame(kid);
		Log::Verbose("{} enters frame{}\n", GetLabel(), frame->GetLabel());
		break;
	}
}

vector3d Body::GetTargetIndicatorPosition() const
{
	return vector3d(0, 0, 0);
}

void Body::SetLabel(const std::string &label)
{
	m_label = label;
	Properties().Set("label", label);
}
