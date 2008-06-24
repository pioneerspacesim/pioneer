#include "libs.h"
#include "Planet.h"
#include "Frame.h"

Planet::Planet(StarSystem::SBody::SubType subtype): Body()
{
	m_radius = 6378135.0;
	m_pos = vector3d(0,0,0);
	m_geom = dCreateSphere(0, m_radius);
	m_subtype = subtype;
}
	
Planet::~Planet()
{
	dGeomDestroy(m_geom);
}

vector3d Planet::GetPosition()
{
	return m_pos;
}

void Planet::SetPosition(vector3d p)
{
	m_pos = p;
	dGeomSetPosition(m_geom, p.x, p.y, p.z);
}

void Planet::TransformToModelCoords(const Frame *camFrame)
{
	vector3d fpos = GetPositionRelTo(camFrame);
	glTranslatef(m_pos[0]+fpos.x, m_pos[1]+fpos.y, m_pos[2]+fpos.z);
}

void Planet::SetRadius(double radius)
{
	m_radius = radius;
	dGeomSphereSetRadius(m_geom, radius);
}

void Planet::Render(const Frame *a_camFrame)
{
	static GLUquadricObj *qobj = NULL;

	if (!qobj) qobj = gluNewQuadric();

	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	
	double rad = m_radius;
	vector3d fpos = GetPositionRelTo(a_camFrame);

	double apparent_size = rad / fpos.Length();
	double len = fpos.Length();

	while (len > 10000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(fpos.x, fpos.y, fpos.z);
	glColor3f(1,1,1);

	if (apparent_size < 0.001) {
		glDisable(GL_LIGHTING);
		glPointSize(1.0);
		glBegin(GL_POINTS);
		glVertex3f(0,0,0);
		glEnd();
		glEnable(GL_LIGHTING);
	} else {
		gluSphere(qobj, rad, 100, 100);
	}
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void Planet::SetFrame(Frame *f)
{
	if (GetFrame()) GetFrame()->RemoveGeom(m_geom);
	Body::SetFrame(f);
	if (f) f->AddGeom(m_geom);
}

