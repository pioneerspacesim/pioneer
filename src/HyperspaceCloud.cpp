#include "libs.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Ship.h"
#include "Serializer.h"
#include "Shader.h"
#include "Space.h"

HyperspaceCloud::HyperspaceCloud(Ship *s, double dueDate, bool isArrival)
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME;
	m_ship = s;
	m_pos = vector3d(0,0,0);
	m_vel = s->GetVelocity();
	m_birthdate = Pi::GetGameTime();
	m_due = dueDate;
	m_isArrival = isArrival;
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

vector3d HyperspaceCloud::GetPosition() const
{
	return m_pos;
}

void HyperspaceCloud::SetPosition(vector3d p)
{
	m_pos = p;
}

void HyperspaceCloud::Save()
{
	using namespace Serializer::Write;
	Body::Save();
	wr_vector3d(m_pos);
	wr_vector3d(m_vel);
	wr_double(m_birthdate);
	wr_double(m_due);
	wr_bool(m_isArrival);
	m_ship->Serialize();
}

void HyperspaceCloud::Load()
{
	using namespace Serializer::Read;
	Body::Load();
	m_pos = rd_vector3d();
	m_vel = rd_vector3d();
	m_birthdate = rd_double();
	m_due = rd_double();
	m_isArrival = rd_bool();
	m_ship = (Ship*)Body::Unserialize();
}

void HyperspaceCloud::PostLoadFixup()
{
	m_ship->PostLoadFixup();
}

void HyperspaceCloud::TimeStepUpdate(const float timeStep)
{
	m_pos += m_vel * timeStep;

	if (m_ship && (m_due < Pi::GetGameTime())) {
		// spawn ship
		m_ship->SetPosition(m_pos);
		m_ship->SetVelocity(m_vel);
		m_ship->SetFrame(GetFrame());
		Space::AddBody(m_ship);
		m_ship->Enable();
		m_ship = 0;
	}
}

void HyperspaceCloud::Render(const Frame *a_camFrame)
{
	Shader::EnableVertexProgram(Shader::VPROG_SIMPLE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glPushMatrix();
	vector3d fpos = GetPositionRelTo(a_camFrame);
	glTranslatef((float)fpos.x, (float)fpos.y, (float)fpos.z);
	if (m_isArrival) {
		glColor4f(1.0,1.0,1.0,0.5);
		gluSphere(Pi::gluQuadric, 25.0, 20, 20);
		glColor4f(.5,.5,1.0,0.5);
		gluSphere(Pi::gluQuadric, 50.0, 20, 20);
		glColor4f(0,0,1.0,0.5);
		gluSphere(Pi::gluQuadric, 100.0, 20, 20);
	} else {
		glColor4f(1.0,1.0,1.0,0.5);
		gluSphere(Pi::gluQuadric, 25.0, 20, 20);
		glColor4f(1.0,.5,.5,0.5);
		gluSphere(Pi::gluQuadric, 50.0, 20, 20);
		glColor4f(1.0,0,0,0.5);
		gluSphere(Pi::gluQuadric, 100.0, 20, 20);
	}
	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	Shader::DisableVertexProgram();
}
