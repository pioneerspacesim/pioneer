#include "libs.h"
#include "Planet.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"

Planet::Planet(StarSystem::SBody::SubType subtype): Body()
{
	m_radius = 6378135.0;
	m_pos = vector3d(0,0,0);
	m_geom = dCreateSphere(0, m_radius);
	dGeomSetData(m_geom, static_cast<Body*>(this));
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

void subdivide(vector3d &v1, vector3d &v2, vector3d &v3, vector3d &v4, int depth)
{
	if (depth) {
		depth--;
		vector3d v5 = v1+v2;
		vector3d v6 = v2+v3;
		vector3d v7 = v3+v4;
		vector3d v8 = v4+v1;
		vector3d v9 = v1+v2+v3+v4;

		v5.Normalize();
		v6.Normalize();
		v7.Normalize();
		v8.Normalize();
		v9.Normalize();

		// XXX wrong wrong wrong wrong. need to do projection turd and stuff
#if 0
		// front-facing
		bool ff1, ff2, ff3, ff4, ff5, ff6, ff7, ff8, ff9;
		const matrix4x4d &r = Pi::world_view->viewingRotation;

		ff1 = (r*v1).z > 0; 
		ff2 = (r*v2).z > 0; 
		ff3 = (r*v3).z > 0; 
		ff4 = (r*v4).z > 0; 
		ff5 = (r*v5).z > 0; 
		ff6 = (r*v6).z > 0; 
		ff7 = (r*v7).z > 0; 
		ff8 = (r*v8).z > 0; 
		ff9 = (r*v9).z > 0; 
#endif
/*		if (ff1 || ff5 || ff9 || ff8)*/ subdivide(v1,v5,v9,v8,depth);
/*		if (ff5 || ff2 || ff6 || ff9)*/ subdivide(v5,v2,v6,v9,depth);
/*		if (ff9 || ff6 || ff3 || ff7)*/ subdivide(v9,v6,v3,v7,depth);
/*		if (ff8 || ff9 || ff7 || ff4)*/ subdivide(v8,v9,v7,v4,depth);
	} else {
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3dv(&v1.x);
		glVertex3dv(&v1.x);
		glNormal3dv(&v2.x);
		glVertex3dv(&v2.x);
		glNormal3dv(&v4.x);
		glVertex3dv(&v4.x);
		glNormal3dv(&v3.x);
		glVertex3dv(&v3.x);
		glEnd();
	}
}

void DrawShittyRoundCube(double radius)
{
	const float mdiff[] = { 0.8, 0.8, 0.5, 1.0 };
	const float mambient[] = { 0.02, 0.02, 0.01, 1.0 };
	const float mdiff2[] = { 0.2, 0.2, 0.8, 0.5 };
	const float mambient2[] = { 0.01, 0.01, 0.04, 0.5 };
	glMaterialfv (GL_FRONT, GL_AMBIENT, mambient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, mdiff);

	vector3d p1(1,1,1);
	vector3d p2(-1,1,1);
	vector3d p3(-1,-1,1);
	vector3d p4(1,-1,1);

	vector3d p5(1,1,-1);
	vector3d p6(-1,1,-1);
	vector3d p7(-1,-1,-1);
	vector3d p8(1,-1,-1);

	p1.Normalize();
	p2.Normalize();
	p3.Normalize();
	p4.Normalize();
	p5.Normalize();
	p6.Normalize();
	p7.Normalize();
	p8.Normalize();

//	p1 *= radius;
//	p2 *= radius;
//	p3 *= radius;
//	p4 *= radius;
//	p5 *= radius;
//	p6 *= radius;
//	p7 *= radius;
//	p8 *= radius;

//	glDisable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	subdivide(p1, p2, p3, p4, 3);
	subdivide(p4, p3, p7, p8, 3);
	subdivide(p1, p4, p8, p5, 3);
	subdivide(p2, p1, p5, p6, 3);
	subdivide(p3, p2, p6, p7, 3);
	subdivide(p8, p7, p6, p5, 3);
	
	glEnable(GL_BLEND);
	glMaterialfv (GL_FRONT, GL_AMBIENT, mambient2);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, mdiff2);
	subdivide(p1, p2, p3, p4, 3);
	glDisable(GL_BLEND);
	
	glDisable(GL_NORMALIZE);
}

void Planet::Render(const Frame *a_camFrame)
{
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
		glScalef(rad,rad,rad);
		DrawShittyRoundCube(1.0f);
//		gluSphere(Pi::gluQuadric, rad, 100, 100);
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

