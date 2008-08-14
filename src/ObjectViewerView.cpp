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
	float fracH = Pi::GetScrHeight() / (float)Pi::GetScrWidth();
	glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	matrix4x4d camRot;
	camRot = matrix4x4d::RotateXMatrix(-DEG2RAD(rot));
	camRot = matrix4x4d::RotateYMatrix(-DEG2RAD(rot)) * camRot;
	vector3d pos = camRot * vector3d(0,0,viewingDist);
		
	float lightPos[4] = { 1, 1, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	Body *body = Pi::player->GetNavTarget();
	if (body) {
		Frame cam_frame(body->GetFrame(), "", Frame::TEMP_VIEWING);
		cam_frame.SetOrientation(camRot);
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
	Body *body = Pi::player->GetNavTarget();
	if(body && body != lastTarget) {
		// Reset view distance for new target.
		viewingDist = body->GetRadius() * 2.0f;
		lastTarget = body;
	}
	snprintf(buf, sizeof(buf), "View dist: %.2f     Object: %s", viewingDist, (body ? body->GetLabel().c_str() : "<none>"));
	m_infoLabel->SetText(buf);
}

