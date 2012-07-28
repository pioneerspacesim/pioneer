#include "libs.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Ship.h"
#include "Serializer.h"
#include "Space.h"
#include "Player.h"
#include "perlin.h"
#include "Lang.h"
#include "Game.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

using namespace Graphics;

HyperspaceCloud::HyperspaceCloud(Ship *s, double dueDate, bool isArrival)
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME |
		  Body::FLAG_LABEL_HIDDEN;
	m_ship = s;
	m_pos = vector3d(0,0,0);
	m_vel = (s ? s->GetVelocity() : vector3d(0.0));
	m_birthdate = Pi::game->GetTime();
	m_due = dueDate;
	SetIsArrival(isArrival);
}

HyperspaceCloud::HyperspaceCloud()
{
	m_ship = 0;
	m_pos = vector3d(0,0,0);
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

vector3d HyperspaceCloud::GetPosition() const
{
	return m_pos;
}

void HyperspaceCloud::SetPosition(vector3d p)
{
	m_pos = p;
}

void HyperspaceCloud::Save(Serializer::Writer &wr, Space *space)
{
	Body::Save(wr, space);
	wr.Vector3d(m_pos);
	wr.Vector3d(m_vel);
	wr.Double(m_birthdate);
	wr.Double(m_due);
	wr.Bool(m_isArrival);
	wr.Bool(m_ship != 0);
	if (m_ship) m_ship->Serialize(wr, space);
}

void HyperspaceCloud::Load(Serializer::Reader &rd, Space *space)
{
	Body::Load(rd, space);
	m_pos = rd.Vector3d();
	m_vel = rd.Vector3d();
	m_birthdate = rd.Double();
	m_due = rd.Double();
	m_isArrival = rd.Bool();
	if (rd.Bool()) {
		m_ship = reinterpret_cast<Ship*>(Body::Unserialize(rd, space));
	}
}

void HyperspaceCloud::PostLoadFixup(Space *space)
{
	if (m_ship) m_ship->PostLoadFixup(space);
}

void HyperspaceCloud::TimeStepUpdate(const float timeStep)
{
	m_pos += m_vel * timeStep;

	if (m_isArrival && m_ship && (m_due < Pi::game->GetTime())) {
		// spawn ship
		// XXX some overlap with Space::DoHyperspaceTo(). should probably all
		// be moved into EvictShip()
		m_ship->SetPosition(m_pos);
		m_ship->SetVelocity(m_vel);
		m_ship->SetRotMatrix(matrix4x4d::Identity());
		m_ship->SetFrame(GetFrame());
		Pi::game->GetSpace()->AddBody(m_ship);
		m_ship->Enable();

		if (Pi::player->GetNavTarget() == this && !Pi::player->GetCombatTarget())
			Pi::player->SetCombatTarget(m_ship, Pi::player->GetSetSpeedTarget() == this);

		m_ship->EnterSystem();

		m_ship = 0;
	}
}

Ship *HyperspaceCloud::EvictShip()
{
	Ship *s = m_ship;
	m_ship = 0;
	return s;
}

static void make_circle_thing(VertexArray &va, float radius, const Color &colCenter, const Color &colEdge)
{
	va.Add(vector3f(0.f, 0.f, 0.f), colCenter);
	for (float ang=0; ang<float(M_PI)*2.f; ang+=0.1f) {
		va.Add(vector3f(radius*sin(ang), radius*cos(ang), 0.0f), colEdge);
	}
	va.Add(vector3f(0.f, radius, 0.f), colEdge);
}

void HyperspaceCloud::UpdateInterpolatedTransform(double alpha)
{
	m_interpolatedTransform = matrix4x4d::Identity();
	const vector3d newPos = GetPosition();
	const vector3d oldPos = newPos - m_vel*Pi::game->GetTimeStep();
	const vector3d p = alpha*newPos + (1.0-alpha)*oldPos;
	m_interpolatedTransform[12] = p.x;
	m_interpolatedTransform[13] = p.y;
	m_interpolatedTransform[14] = p.z;
}

void HyperspaceCloud::Render(Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	renderer->SetBlendMode(BLEND_ALPHA_ONE);
	glPushMatrix();

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(float(viewCoords.x), float(viewCoords.y), float(viewCoords.z));

	// face the camera dammit
	vector3d zaxis = viewCoords.NormalizedSafe();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	renderer->SetTransform(trans * rot);

	// precise to the rendered frame (better than PHYSICS_HZ granularity)
	double preciseTime = Pi::game->GetTime() + Pi::GetGameTickAlpha()*Pi::game->GetTimeStep();

	float radius = 1000.0f + 200.0f*float(noise(10.0*preciseTime, 0, 0));
	VertexArray va(ATTRIB_POSITION | ATTRIB_DIFFUSE);
	if (m_isArrival) {
		make_circle_thing(va, radius, Color(1.0,1.0,1.0,1.0), Color(0.0,0.0,1.0,0.0));
	} else {
		make_circle_thing(va, radius, Color(1.0,1.0,1.0,1.0), Color(1.0,0.0,0.0,0.0));
	}
	renderer->DrawTriangles(&va, 0, TRIANGLE_FAN);
	renderer->SetBlendMode(BLEND_SOLID);
	glPopMatrix();
}
