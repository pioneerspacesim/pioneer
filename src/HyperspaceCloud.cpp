#include "libs.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Ship.h"
#include "Serializer.h"
#include "Shader.h"

HyperspaceCloud::HyperspaceCloud(Ship *s, double dueDate)
{
	m_ship = s;
	m_pos = vector3d(0,0,0);
	m_birthdate = Pi::GetGameTime();
	m_due = dueDate;
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
	wr_double(m_birthdate);
	wr_double(m_due);
	m_ship->Serialize();
}

void HyperspaceCloud::Load()
{
	using namespace Serializer::Read;
	Body::Load();
	m_pos = rd_vector3d();
	m_birthdate = rd_double();
	m_due = rd_double();
	m_ship = (Ship*)Body::Unserialize();
}

void HyperspaceCloud::PostLoadFixup()
{
	m_ship->PostLoadFixup();
}

void HyperspaceCloud::Render(const Frame *a_camFrame)
{
	Shader::EnableVertexProgram(Shader::VPROG_SIMPLE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glPushMatrix();
	vector3d fpos = GetPositionRelTo(a_camFrame);
	glTranslatef((float)fpos.x, (float)fpos.y, (float)fpos.z);
	glColor4f(1.0,1.0,1.0,0.5);
	gluSphere(Pi::gluQuadric, 25.0, 20, 20);
	glColor4f(.5,.5,1.0,0.5);
	gluSphere(Pi::gluQuadric, 50.0, 20, 20);
	glColor4f(0,0,1.0,0.5);
	gluSphere(Pi::gluQuadric, 100.0, 20, 20);
	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	Shader::DisableVertexProgram();
}
