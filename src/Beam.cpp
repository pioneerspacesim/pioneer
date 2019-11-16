// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Beam.h"

#include "Frame.h"
#include "Game.h"
#include "GameSaveError.h"
#include "Json.h"
#include "LuaEvent.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "Planet.h"
#include "Sfx.h"
#include "Ship.h"
#include "Space.h"
#include "SpawnTastyStuff.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace {
	static float lifetime = 0.1f;
}

std::unique_ptr<Graphics::VertexArray> Beam::s_sideVerts;
std::unique_ptr<Graphics::VertexArray> Beam::s_glowVerts;
std::unique_ptr<Graphics::Material> Beam::s_sideMat;
std::unique_ptr<Graphics::Material> Beam::s_glowMat;
Graphics::RenderState *Beam::s_renderState = nullptr;

void Beam::BuildModel()
{
	//set up materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	s_sideMat.reset(Pi::renderer->CreateMaterial(desc));
	s_glowMat.reset(Pi::renderer->CreateMaterial(desc));
	s_sideMat->texture0 = Graphics::TextureBuilder::Billboard("textures/beam_l.dds").GetOrCreateTexture(Pi::renderer, "billboard");
	s_glowMat->texture0 = Graphics::TextureBuilder::Billboard("textures/projectile_w.dds").GetOrCreateTexture(Pi::renderer, "billboard");

	//zero at projectile position
	//+x down
	//+y right
	//+z forwards (or projectile direction)
	const float w = 0.5f;

	vector3f one(0.f, -w, 0.f); //top left
	vector3f two(0.f, w, 0.f); //top right
	vector3f three(0.f, w, -1.f); //bottom right
	vector3f four(0.f, -w, -1.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	s_sideVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0, 24));
	s_glowVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0, 240));

	//add four intersecting planes to create a volumetric effect
	for (int i = 0; i < 4; i++) {
		s_sideVerts->Add(one, topLeft);
		s_sideVerts->Add(two, topRight);
		s_sideVerts->Add(three, botRight);

		s_sideVerts->Add(three, botRight);
		s_sideVerts->Add(four, botLeft);
		s_sideVerts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create quads for viewing on end
	static const float gw = 0.5f;
	float gz = -0.1f;

	for (int i = 0; i < 40; i++) {
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);
		s_glowVerts->Add(vector3f(-gw, gw, gz), topRight);
		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);

		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);
		s_glowVerts->Add(vector3f(gw, -gw, gz), botLeft);
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);

		gz -= 0.02f; // as they move back
	}

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.cullMode = Graphics::CULL_NONE;
	s_renderState = Pi::renderer->CreateRenderState(rsd);
}

void Beam::FreeModel()
{
	s_sideMat.reset();
	s_glowMat.reset();
	s_sideVerts.reset();
	s_glowVerts.reset();
}

Beam::Beam(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dir) :
	Body(),
	m_age(0),
	m_active(true)
{
	if (!s_sideMat) BuildModel();
	m_flags |= FLAG_DRAW_LAST;

	m_parent = parent;
	m_dir = dir;
	m_baseDam = prData.damage;
	m_length = prData.length;
	m_mining = prData.mining;
	m_color = prData.color;
	SetFrame(parent->GetFrame());

	SetOrient(parent->GetOrient());
	SetPosition(pos);
	m_baseVel = baseVel;
	SetClipRadius(GetRadius());
	SetPhysRadius(GetRadius());
}

Beam::Beam(const Json &jsonObj, Space *space) :
	Body(jsonObj, space),
	m_active(true)
{
	if (!s_sideMat) BuildModel();
	m_flags |= FLAG_DRAW_LAST;

	try {
		Json projectileObj = jsonObj["projectile"];

		JsonToVector(&m_dir, projectileObj["dir"]);
		m_baseDam = projectileObj["base_dam"];
		m_length = projectileObj["length"];
		m_mining = projectileObj["mining"];
		m_age = projectileObj["age"];
		JsonToColor(&m_color, projectileObj["color"]);
		m_parentIndex = projectileObj["index_for_body"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

Beam::~Beam()
{
}

void Beam::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json projectileObj({}); // Create JSON object to contain projectile data.

	projectileObj["dir"] = m_dir;
	projectileObj["base_dam"] = m_baseDam;
	projectileObj["length"] = m_length;
	projectileObj["mining"] = m_mining;
	projectileObj["color"] = m_color;
	projectileObj["age"] = m_age;
	projectileObj["index_for_body"] = space->GetIndexForBody(m_parent);

	jsonObj["projectile"] = projectileObj; // Add projectile object to supplied object.
}

void Beam::PostLoadFixup(Space *space)
{
	Body::PostLoadFixup(space);
	m_parent = space->GetBodyByIndex(m_parentIndex);
}

void Beam::UpdateInterpTransform(double alpha)
{
	m_interpOrient = GetOrient();
	const vector3d oldPos = GetPosition() - (m_baseVel * Pi::game->GetTimeStep());
	m_interpPos = alpha * GetPosition() + (1.0 - alpha) * oldPos;
}

void Beam::NotifyRemoved(const Body *const removedBody)
{
	if (m_parent == removedBody)
		m_parent = nullptr;
}

void Beam::TimeStepUpdate(const float timeStep)
{
	// Laser pulse's do not age well!
	m_age += timeStep;
	if (m_age > lifetime)
		Pi::game->GetSpace()->KillBody(this);
	SetPosition(GetPosition() + (m_baseVel * double(timeStep)));
}

float Beam::GetDamage() const
{
	return m_baseDam;
}

double Beam::GetRadius() const
{
	return sqrt(m_length * m_length);
}

void Beam::StaticUpdate(const float timeStep)
{
	PROFILE_SCOPED()
	// This is just to stop it from hitting things repeatedly, it's dead in effect but still rendered
	if (!m_active)
		return;

	CollisionContact c;
	GetFrame()->GetCollisionSpace()->TraceRay(GetPosition(), m_dir.Normalized(), m_length, &c, static_cast<ModelBody *>(m_parent)->GetGeom());

	if (c.userData1) {
		Object *o = static_cast<Object *>(c.userData1);

		if (o->IsType(Object::CITYONPLANET)) {
			Pi::game->GetSpace()->KillBody(this);
		} else if (o->IsType(Object::BODY)) {
			Body *hit = static_cast<Body *>(o);
			if (hit != m_parent) {
				hit->OnDamage(m_parent, GetDamage(), c);
				m_active = false;
				if (hit->IsType(Object::SHIP))
					LuaEvent::Queue("onShipHit", dynamic_cast<Ship *>(hit), dynamic_cast<Body *>(m_parent));
			}
		}
	}

	if (m_mining) {
		// need to test for terrain hit
		if (GetFrame()->GetBody() && GetFrame()->GetBody()->IsType(Object::PLANET)) {
			Planet *const planet = static_cast<Planet *>(GetFrame()->GetBody());
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				// hit the fucker
				if (planet->SystemBodyWrapper::IsType(GalaxyEnums::BodyType::TYPE_PLANET_ASTEROID)) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), planet->GetSystemBody(), n * terrainHeight + 5.0 * n);
					SfxManager::Add(this, TYPE_EXPLOSION);
				}
				m_active = false;
			}
		}
	}
}

void Beam::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	PROFILE_SCOPED()
	const vector3d _from = viewTransform * GetInterpPosition();
	const vector3d _to = viewTransform * (GetInterpPosition() + (-m_dir));
	const vector3d _dir = _to - _from;
	const vector3f from(&_from.x);
	const vector3f dir = vector3f(_dir).Normalized();

	vector3f v1, v2;
	matrix4x4f m = matrix4x4f::Identity();
	v1.x = dir.y;
	v1.y = dir.z;
	v1.z = dir.x;
	v2 = v1.Cross(dir).Normalized();
	v1 = v2.Cross(dir);
	m[0] = v1.x;
	m[4] = v2.x;
	m[8] = dir.x;
	m[1] = v1.y;
	m[5] = v2.y;
	m[9] = dir.y;
	m[2] = v1.z;
	m[6] = v2.z;
	m[10] = dir.z;

	m[12] = from.x;
	m[13] = from.y;
	m[14] = from.z;

	// increase visible size based on distance from camera, z is always negative
	// allows them to be smaller while maintaining visibility for game play
	const float dist_scale = float(viewCoords.z / -500);
	const float length = m_length + dist_scale;
	const float width = 1.0f + dist_scale;

	renderer->SetTransform(m * matrix4x4f::ScaleMatrix(width, width, length));

	Color color = m_color;
	// fade them out as they age so they don't suddenly disappear
	// this matches the damage fall-off calculation
	const float base_alpha = 1.0f;
	// fade out side quads when viewing nearly edge on
	vector3f view_dir = vector3f(viewCoords).Normalized();
	color.a = (base_alpha * (1.f - powf(fabs(dir.Dot(view_dir)), length))) * 255;

	if (color.a > 3) {
		s_sideMat->diffuse = color;
		renderer->DrawTriangles(s_sideVerts.get(), s_renderState, s_sideMat.get());
	}

	// fade out glow quads when viewing nearly edge on
	// these and the side quads fade at different rates
	// so that they aren't both at the same alpha as that looks strange
	color.a = (base_alpha * powf(fabs(dir.Dot(view_dir)), width)) * 255;

	if (color.a > 3) {
		s_glowMat->diffuse = color;
		renderer->DrawTriangles(s_glowVerts.get(), s_renderState, s_glowMat.get());
	}
}

// static
void Beam::Add(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dir)
{
	Beam *p = new Beam(parent, prData, pos, baseVel, dir);
	Pi::game->GetSpace()->AddBody(p);
}
