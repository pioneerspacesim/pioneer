#include "ObjectViewerView.h"
#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"

ObjectViewerView::ObjectViewerView(): View()
{
	SetTransparency(true);
	viewingDist = 1000.0f;
	m_camRot = matrix4x4d::Identity();
	
	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, 2);
}

void ObjectViewerView::Draw3D()
{
	static float rot;
	rot += 0.1;
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float znear = 10.0f;
	float zfar = 1000000.0f;
	float fracH = znear / Pi::GetScrAspect();
	glFrustum(-znear, znear, -fracH, fracH, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_LIGHT0);

	if (Pi::MouseButtonState(3)) {
		int m[2];
		Pi::GetMouseMotion(m);
		m_camRot = m_camRot * matrix4x4d::RotateXMatrix(0.002*m[1]) *
				matrix4x4d::RotateYMatrix(0.002*m[0]);
	}
	vector3d pos = m_camRot * vector3d(0,0,viewingDist);
		
	float lightPos[4] = { .577, .577, .577, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	Body *body = Pi::player->GetNavTarget();
	if (body) {
		Frame cam_frame(body->GetFrame(), "", Frame::TEMP_VIEWING);
		cam_frame.SetOrientation(m_camRot);
		cam_frame.SetPosition(body->GetPosition()+pos);
		body->Render(&cam_frame);
		body->GetFrame()->RemoveChild(&cam_frame);
	}
}

void ObjectViewerView::OnSwitchTo()
{
	m_camRot = matrix4x4d::Identity();
}

void ObjectViewerView::Update()
{
	if (Pi::KeyState(SDLK_EQUALS)) viewingDist *= 0.99;
	if (Pi::KeyState(SDLK_MINUS)) viewingDist *= 1.01;
	viewingDist = CLAMP(viewingDist, 10, 1e12);

	char buf[128];
	Body *body = Pi::player->GetNavTarget();
	if(body && body != lastTarget) {
		// Reset view distance for new target.
		viewingDist = body->GetBoundingRadius() * 2.0f;
		lastTarget = body;
	}
	snprintf(buf, sizeof(buf), "View dist: %.2f     Object: %s", viewingDist, (body ? body->GetLabel().c_str() : "<none>"));
	m_infoLabel->SetText(buf);
}

