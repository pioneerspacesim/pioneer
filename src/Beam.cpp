// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "Beam.h"
#include "Frame.h"
#include "galaxy/StarSystem.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "CargoBody.h"
#include "Planet.h"
#include "Sfx.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"
#include "LuaEvent.h"
#include "LuaUtils.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"
#include "json/JsonUtils.h"

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
	s_sideMat->texture0 = Graphics::TextureBuilder::Billboard("textures/projectile_l.png").GetOrCreateTexture(Pi::renderer, "billboard");
	s_glowMat->texture0 = Graphics::TextureBuilder::Billboard("textures/projectile_w.png").GetOrCreateTexture(Pi::renderer, "billboard");

	//zero at projectile position
	//+x down
	//+y right
	//+z forwards (or projectile direction)
	const float w = 0.5f;

	vector3f one(0.f, -w, 0.f); //top left
	vector3f two(0.f,  w, 0.f); //top right
	vector3f three(0.f,  w, -1.f); //bottom right
	vector3f four(0.f, -w, -1.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	s_sideVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));
	s_glowVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));

	//add four intersecting planes to create a volumetric effect
	for (int i=0; i < 4; i++) {
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
	float gw = 0.5f;
	float gz = -0.1f;

	for (int i=0; i < 4; i++) {
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);
		s_glowVerts->Add(vector3f(-gw, gw, gz), topRight);
		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);

		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);
		s_glowVerts->Add(vector3f(gw, -gw, gz), botLeft);
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);

		gw -= 0.1f; // they get smaller
		gz -= 0.2f; // as they move back
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

Beam::Beam(): Body()
{
	if (!s_sideMat) BuildModel();
	SetOrient(matrix3x3d::Identity());
	m_baseDam = 0;
	m_length = 0;
	m_mining = false;
	m_parent = 0;
	m_flags |= FLAG_DRAW_LAST;
	m_canKill = false;
}

Beam::~Beam()
{
}

void Beam::SaveToJson(Json::Value &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json::Value projectileObj(Json::objectValue); // Create JSON object to contain projectile data.

	VectorToJson(projectileObj, m_dir, "dir");
	projectileObj["base_dam"] = FloatToStr(m_baseDam);
	projectileObj["length"] = FloatToStr(m_length);
	projectileObj["mining"] = m_mining;
	ColorToJson(projectileObj, m_color, "color");
	projectileObj["index_for_body"] = space->GetIndexForBody(m_parent);

	jsonObj["projectile"] = projectileObj; // Add projectile object to supplied object.
}

void Beam::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	Body::LoadFromJson(jsonObj, space);

	if (!jsonObj.isMember("projectile")) throw SavedGameCorruptException();
	Json::Value projectileObj = jsonObj["projectile"];

	if (!projectileObj.isMember("dir")) throw SavedGameCorruptException();
	if (!projectileObj.isMember("base_dam")) throw SavedGameCorruptException();
	if (!projectileObj.isMember("length")) throw SavedGameCorruptException();
	if (!projectileObj.isMember("mining")) throw SavedGameCorruptException();
	if (!projectileObj.isMember("color")) throw SavedGameCorruptException();
	if (!projectileObj.isMember("index_for_body")) throw SavedGameCorruptException();

	JsonToVector(&m_dir, projectileObj, "dir");
	m_baseDam = StrToFloat(projectileObj["base_dam"].asString());
	m_length = StrToFloat(projectileObj["length"].asString());
	m_mining = projectileObj["mining"].asBool();
	JsonToColor(&m_color, projectileObj, "color");
	m_parentIndex = projectileObj["index_for_body"].asInt();
}

void Beam::PostLoadFixup(Space *space)
{
	Body::PostLoadFixup(space);
	m_parent = space->GetBodyByIndex(m_parentIndex);
}

void Beam::NotifyRemoved(const Body* const removedBody)
{
	if (m_parent == removedBody) 
		m_parent = nullptr;
}

void Beam::TimeStepUpdate(const float timeStep)
{
	if(m_canKill) {
		// Laser pulse's only last for one frame
		Pi::game->GetSpace()->KillBody(this);
	} else {
		m_canKill = true;
	}
}

float Beam::GetDamage() const
{
	return m_baseDam;
}

double Beam::GetRadius() const
{
	return sqrt(m_length*m_length);
}

static void MiningLaserSpawnTastyStuff(Frame *f, const SystemBody *asteroid, const vector3d &pos)
{
	lua_State *l = Lua::manager->GetLuaState();
	pi_lua_import(l, "Equipment");
	LuaTable cargo_types = LuaTable(l, -1).Sub("cargo");
	if (20*Pi::rng.Fixed() < asteroid->GetMetallicityAsFixed()) {
		cargo_types.Sub("precious_metals");
	} else if (8*Pi::rng.Fixed() < asteroid->GetMetallicityAsFixed()) {
		cargo_types.Sub("metal_alloys");
	} else if (Pi::rng.Fixed() < asteroid->GetMetallicityAsFixed()) {
		cargo_types.Sub("metal_ore");
	} else if (Pi::rng.Fixed() < fixed(1,2)) {
		cargo_types.Sub("water");
	} else {
		cargo_types.Sub("rubbish");
	}
	CargoBody *cargo = new CargoBody(LuaRef(l, -1));
	lua_pop(l, 3);
	cargo->SetFrame(f);
	cargo->SetPosition(pos);
	const double x = Pi::rng.Double();
	vector3d dir = pos.Normalized();
	dir.ArbRotate(vector3d(x, 1-x, 0), Pi::rng.Double()-.5);
	cargo->SetVelocity(Pi::rng.Double(100.0,200.0) * dir);
	Pi::game->GetSpace()->AddBody(cargo);
}

void Beam::StaticUpdate(const float timeStep)
{
	PROFILE_SCOPED()

	CollisionContact c;
	GetFrame()->GetCollisionSpace()->TraceRay(GetPosition(), m_dir.Normalized(), m_length, &c);

	if (c.userData1) {
		Object *o = static_cast<Object*>(c.userData1);

		if (o->IsType(Object::CITYONPLANET)) {
			Pi::game->GetSpace()->KillBody(this);
		}
		else if (o->IsType(Object::BODY)) {
			Body *hit = static_cast<Body*>(o);
			if (hit != m_parent) {
				hit->OnDamage(m_parent, GetDamage(), c);
				Pi::game->GetSpace()->KillBody(this);
				if (hit->IsType(Object::SHIP))
					LuaEvent::Queue("onShipHit", dynamic_cast<Ship*>(hit), dynamic_cast<Body*>(m_parent));
			}
		}
	}
	if (m_mining) {
		// need to test for terrain hit
		if (GetFrame()->GetBody() && GetFrame()->GetBody()->IsType(Object::PLANET)) {
			Planet *const planet = static_cast<Planet*>(GetFrame()->GetBody());
			const SystemBody *b = planet->GetSystemBody();
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				// hit the fucker
				if (b->GetType() == SystemBody::TYPE_PLANET_ASTEROID) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), b, n*terrainHeight + 5.0*n);
					SfxManager::Add(this, TYPE_EXPLOSION);
				}
				Pi::game->GetSpace()->KillBody(this);
			}
		}
	}
}

void Beam::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	PROFILE_SCOPED()
	vector3d _from = viewTransform * GetInterpPosition();
	vector3d _to = viewTransform * (GetInterpPosition() + m_dir);
	vector3d _dir = _to - _from;
	vector3f from(&_from.x);
	vector3f dir = vector3f(_dir).Normalized();

	vector3f v1, v2;
	matrix4x4f m = matrix4x4f::Identity();
	v1.x = dir.y; v1.y = dir.z; v1.z = dir.x;
	v2 = v1.Cross(dir).Normalized();
	v1 = v2.Cross(dir);
	m[0] = v1.x; m[4] = v2.x; m[8] = dir.x;
	m[1] = v1.y; m[5] = v2.y; m[9] = dir.y;
	m[2] = v1.z; m[6] = v2.z; m[10] = dir.z;

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
void Beam::Add(Body *parent, const vector3d &pos, const vector3d &dir, const double length, const float damage, const bool mining, const Color& color)
{
	Beam *p = new Beam();
	p->m_parent = parent;
	p->m_dir = dir;
	p->m_baseDam = damage;
	p->m_length = length;
	p->m_mining = mining;
	p->m_color = color;
	p->SetFrame(parent->GetFrame());

	p->SetOrient(parent->GetOrient());
	p->SetPosition(pos);
	p->SetClipRadius(p->GetRadius());
	p->SetPhysRadius(p->GetRadius());
	Pi::game->GetSpace()->AddBody(p);
}
