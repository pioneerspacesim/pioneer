#include "libs.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Ship.h"
#include "Serializer.h"
#include "Shader.h"
#include "Space.h"
#include "perlin.h"

HyperspaceCloud::HyperspaceCloud(Ship *s, double dueDate, bool isArrival)
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME |
		  Body::FLAG_LABEL_HIDDEN;
	m_ship = s;
	m_pos = vector3d(0,0,0);
	m_vel = (s ? s->GetVelocity() : vector3d(0.0));
	m_birthdate = Pi::GetGameTime();
	m_due = dueDate;
	m_id = Pi::rng.Int32();
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
	SetLabel(isArrival ? "Hyperspace arrival cloud" : "Hyperspace departure cloud");
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
	wr_int(m_id);
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
	if (!IsOlderThan(9)) m_id = rd_int();
	else m_id = Pi::rng.Int32();
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

static void make_circle_thing(float radius, const Color &colCenter, const Color &colEdge)
{
	glColor4fv(colCenter);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	glColor4fv(colEdge);
	for (float ang=0; ang<M_PI*2.0; ang+=0.1) {
		glVertex3f(radius*sin(ang), radius*cos(ang), 0.0f);
	}
	glVertex3f(0, radius, 0.0f);
	glEnd();
}

void HyperspaceCloud::Render(const Frame *a_camFrame)
{
	Shader::EnableVertexProgram(Shader::VPROG_SIMPLE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glPushMatrix();
	vector3d fpos = GetPositionRelTo(a_camFrame);
	glTranslatef((float)fpos.x, (float)fpos.y, (float)fpos.z);
	
	// face the camera dammit
	vector3d zaxis = fpos.Normalized();
	vector3d xaxis = vector3d::Cross(vector3d(0,1,0), zaxis).Normalized();
	vector3d yaxis = vector3d::Cross(zaxis,xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	glMultMatrixd(&rot[0]);

	float radius = 1000.0f + 200.0f*noise(10.0*Pi::GetGameTime(), (double)(m_id&0xff), 0);
	if (m_isArrival) {
		make_circle_thing(radius, Color(1.0,1.0,1.0,1.0), Color(0.0,0.0,1.0,0.0));
	} else {
		make_circle_thing(radius, Color(1.0,1.0,1.0,1.0), Color(1.0,0.0,0.0,0.0));
	}
	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	Shader::DisableVertexProgram();
}
