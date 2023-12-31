// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "HyperspaceCloud.h"

#include "Game.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Lang.h"
#include "Pi.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "perlin.h"

using namespace Graphics;

/** How long does a hyperspace cloud last for? 2 Days? */
#define HYPERCLOUD_DURATION (60.0 * 60.0 * 24.0 * 2.0)

// TODO: having these as a static pointer isn't great; find a better way to handle this
std::unique_ptr<Graphics::Material> HyperspaceCloud::s_cloudMat;
std::unique_ptr<Graphics::MeshObject> HyperspaceCloud::s_cloudMeshArriving;
std::unique_ptr<Graphics::MeshObject> HyperspaceCloud::s_cloudMeshLeaving;

HyperspaceCloud::HyperspaceCloud(Ship *s, double dueDate, bool isArrival) :
	m_isBeingKilled(false)
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME |
		Body::FLAG_LABEL_HIDDEN;
	m_ship = s;
	SetPhysRadius(0.0);
	SetClipRadius(1200.0);
	m_vel = (s ? s->GetVelocity() : vector3d(0.0));
	m_birthdate = Pi::game->GetTime();
	m_due = dueDate;
	SetIsArrival(isArrival);
}

HyperspaceCloud::HyperspaceCloud(const Json &jsonObj, Space *space) :
	Body(jsonObj, space),
	m_isBeingKilled(false)
{
	try {
		Json hyperspaceCloudObj = jsonObj["hyperspace_cloud"];

		m_vel = hyperspaceCloudObj["vel"];
		m_birthdate = hyperspaceCloudObj["birth_date"];
		m_due = hyperspaceCloudObj["due"];
		m_isArrival = hyperspaceCloudObj["is_arrival"];

		m_ship = nullptr;
		if (hyperspaceCloudObj["ship"].is_object()) {
			Json shipObj = hyperspaceCloudObj["ship"];
			m_ship = static_cast<Ship *>(Body::FromJson(shipObj, space));
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

HyperspaceCloud::~HyperspaceCloud()
{
	if (m_ship) delete m_ship;
}

void HyperspaceCloud::SetIsArrival(bool isArrival)
{
	m_isArrival = isArrival;
	SetLabel(isArrival ? Lang::HYPERSPACE_ARRIVAL_CLOUD : Lang::HYPERSPACE_DEPARTURE_CLOUD);
}

void HyperspaceCloud::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json hyperspaceCloudObj = Json::object(); // Create JSON object to contain hyperspace cloud data.

	hyperspaceCloudObj["vel"] = m_vel;
	hyperspaceCloudObj["birth_date"] = m_birthdate;
	hyperspaceCloudObj["due"] = m_due;
	hyperspaceCloudObj["is_arrival"] = m_isArrival;
	if (m_ship) {
		Json shipObj = Json::object(); // Create JSON object to contain ship data.
		m_ship->ToJson(shipObj, space);
		hyperspaceCloudObj["ship"] = shipObj; // Add ship object to hyperpace cloud object.
	}

	jsonObj["hyperspace_cloud"] = hyperspaceCloudObj; // Add hyperspace cloud object to supplied object.
}

void HyperspaceCloud::PostLoadFixup(Space *space)
{
	Body::PostLoadFixup(space);
	if (m_ship) m_ship->PostLoadFixup(space);
}

void HyperspaceCloud::TimeStepUpdate(const float timeStep)
{
	if (m_isBeingKilled)
		return;

	SetPosition(GetPosition() + m_vel * timeStep);

	if (m_isArrival && m_ship && (m_due < Pi::game->GetTime())) {
		// spawn ship
		// XXX some overlap with Space::DoHyperspaceTo(). should probably all
		// be moved into EvictShip()
		m_ship->SetPosition(GetPosition());
		m_ship->SetVelocity(m_vel);
		m_ship->SetOrient(matrix3x3d::Identity());
		m_ship->SetFrame(GetFrame());
		Pi::game->GetSpace()->AddBody(m_ship);

		if (Pi::player->GetNavTarget() == this && !Pi::player->GetCombatTarget())
			Pi::player->SetCombatTarget(m_ship, Pi::player->GetFollowTarget() == this);

		m_ship->EnterSystem();

		m_ship = nullptr;
	}

	// cloud expiration
	if (m_birthdate + HYPERCLOUD_DURATION <= Pi::game->GetTime()) {
		Pi::game->RemoveHyperspaceCloud(this);
		Pi::game->GetSpace()->KillBody(this);
		m_isBeingKilled = true;
	}
}

Ship *HyperspaceCloud::EvictShip()
{
	Ship *s = m_ship;
	m_ship = nullptr;
	return s;
}

static void make_circle_thing(VertexArray &va, float radius, const Color &colCenter, const Color &colEdge)
{
	va.Add(vector3f(0.f, 0.f, 0.f), colCenter);
	for (float ang = 0; ang < float(M_PI) * 2.f; ang += 0.1f) {
		va.Add(vector3f(radius * sin(ang), radius * cos(ang), 0.0f), colEdge);
	}
	va.Add(vector3f(0.f, radius, 0.f), colEdge);
}

void HyperspaceCloud::UpdateInterpTransform(double alpha)
{
	m_interpOrient = matrix3x3d::Identity();
	const vector3d oldPos = GetPosition() - m_vel * Pi::game->GetTimeStep();
	m_interpPos = alpha * GetPosition() + (1.0 - alpha) * oldPos;
}

void HyperspaceCloud::Render(Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (m_isBeingKilled)
		return;

	if (!s_cloudMat)
		InitGraphics(renderer);

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(float(viewCoords.x), float(viewCoords.y), float(viewCoords.z));

	// precise to the rendered frame (better than PHYSICS_HZ granularity)
	const double preciseTime = Pi::game->GetTime() + Pi::GetGameTickAlpha() * Pi::game->GetTimeStep();

	// Flickering gradient circle, departure clouds are red and arrival clouds blue
	// XXX could just alter the scale instead of recreating the model
	const float scale = 1.0f + 0.2f * float(noise(vector3d(10.0 * preciseTime, 0, 0)));

	// face the camera dammit
	vector3d zaxis = viewCoords.NormalizedSafe();
	vector3d xaxis = vector3d(0, 1, 0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();
	renderer->SetTransform(matrix4x4f(trans * rot * matrix4x4d::ScaleMatrix(scale)));

	renderer->DrawMesh(m_isArrival ? s_cloudMeshArriving.get() : s_cloudMeshLeaving.get(), s_cloudMat.get());
}

void HyperspaceCloud::InitGraphics(Graphics::Renderer *renderer)
{
	Graphics::MaterialDescriptor desc;
	desc.vertexColors = true;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.primitiveType = Graphics::TRIANGLE_FAN;
	s_cloudMat.reset(renderer->CreateMaterial("unlit", desc, rsd));

	Graphics::VertexArray vertices(ATTRIB_POSITION | ATTRIB_DIFFUSE);

	const Color edgeArrivingColour(Color::BLUE, 0); // alpha needs to be zero'd
	make_circle_thing(vertices, 1000.f, Color::WHITE, edgeArrivingColour);
	s_cloudMeshArriving.reset(renderer->CreateMeshObjectFromArray(&vertices));

	vertices.Clear();

	const Color edgeLeavingColour(Color::RED, 0); // alpha needs to be zero'd
	make_circle_thing(vertices, 1000.f, Color::WHITE, edgeLeavingColour);
	s_cloudMeshLeaving.reset(renderer->CreateMeshObjectFromArray(&vertices));
}
