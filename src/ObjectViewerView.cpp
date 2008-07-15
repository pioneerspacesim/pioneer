#include "ObjectViewerView.h"
#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"

ObjectViewerView::ObjectViewerView(): View()
{
	SetTransparency(true);
	viewingRotation = matrix4x4d::Identity();
	viewingDist = 1000.0f;
	
	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, 2);
}

/*
vector3d Player::GetExternalViewTranslation()
{
	vector3d p = vector3d(0, 0, m_external_view_dist);
	p = matrix4x4d::RotateXMatrix(-DEG_2_RAD*m_external_view_rotx) * p;
	p = matrix4x4d::RotateYMatrix(-DEG_2_RAD*m_external_view_roty) * p;
	matrix4x4d m;
	GetRotMatrix(m);
	p = m*p;
//	printf("%f,%f,%f\n", p.x, p.y, p.z);
	return p;
}

void Player::ApplyExternalViewRotation()
{
//	glTranslatef(0, 0, m_external_view_dist);
	glRotatef(-m_external_view_rotx, 1, 0, 0);
	glRotatef(-m_external_view_roty, 0, 1, 0);
}
*/
#define DEG_2_RAD	0.0174532925
void ObjectViewerView::Draw3D()
{
	static float rot;
	rot+= 0.1;
	glClearColor(0,0,0.1,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fracH = Pi::GetScrHeight() / (float)Pi::GetScrWidth();
	glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3d pos = vector3d(0,0,viewingDist);
//	p = matrix4x4d::RotateXMatrix(-DEG_2_RAD*m_external_view_rotx) * p;
	pos = matrix4x4d::RotateYMatrix(-DEG_2_RAD*rot) * pos;
	pos = matrix4x4d::RotateXMatrix(-DEG_2_RAD*rot) * pos;
		
/*	float lightPos[4];
	lightPos[0] = 0;
	lightPos[1] = 0;
	lightPos[2] = 0;
	lightPos[3] = 0;
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
*/	
	// sbre rendering (see ModelBody.cpp) uses this...
	glRotatef(-rot,0,1,0);
	glRotatef(-rot,1,0,0);
	//Pi::world_view->viewingRotation = matrix4x4d::Identity();
	glGetDoublev (GL_MODELVIEW_MATRIX, &Pi::world_view->viewingRotation[0]);
	
	Body *body = Pi::player->GetNavTarget();
	if (body) {
		Frame cam_frame(body->GetFrame(), "", Frame::TEMP_VIEWING);
		cam_frame.SetPosition(body->GetPosition()+pos);
		body->Render(&cam_frame);
		body->GetFrame()->RemoveChild(&cam_frame);
	}
}

void ObjectViewerView::Update()
{
	if (Pi::KeyState(SDLK_EQUALS)) viewingDist *= 0.99;
	if (Pi::KeyState(SDLK_MINUS)) viewingDist *= 1.01;
	viewingDist = CLAMP(viewingDist, 10, 1e10);

	char buf[128];
	snprintf(buf, sizeof(buf), "View dist: %.2f", viewingDist);
	m_infoLabel->SetText(buf);
}

