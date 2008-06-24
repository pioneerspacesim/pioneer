#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"

static const float lightCol[] = { 1,1,.9,0 };

#define BG_STAR_MAX	2000

WorldView::WorldView(): View()
{
	SetTransparency(true);

	m_hyperspaceButton = new Gui::ImageButton("icons/hyperspace_f8.png");
	m_hyperspaceButton->SetShortcut(SDLK_F8, KMOD_NONE);
	m_hyperspaceButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickHyperspace));
	m_rightButtonBar->Add(m_hyperspaceButton, 66, 2);
	
	m_bgstarsDlist = glGenLists(1);

	glNewList(m_bgstarsDlist, GL_COMPILE);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPointSize(1.0);
	glBegin(GL_POINTS);
	for (int i=0; i<BG_STAR_MAX; i++) {
		float col = 0.2+Pi::rng(0.8);
		glColor3f(col, col, col);
		glVertex3f(1000-Pi::rng(2000.0), 1000-Pi::rng(2000.0), 1000-Pi::rng(2000.0));
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glEndList();
}

void WorldView::OnClickHyperspace()
{
	StarSystem *s = Pi::GetSelectedSystem();
	if (s /* && isn't current system */ ) {
		printf("Hyperspace!!!!!! zoooooooom!!!!!!!\n");
		Pi::HyperspaceTo(s);
	}
}

void WorldView::Draw3D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// why the hell do i give these functions such big names..
	glFrustum(-Pi::GetScrWidth()*.5, Pi::GetScrWidth()*.5,
		  -Pi::GetScrHeight()*.5, Pi::GetScrHeight()*.5,
		   Pi::GetScrWidth()*.5, 100000);
	glDepthRange (-10, -100000);
	glMatrixMode(GL_MODELVIEW);

	// make temporary camera frame at player
	Frame *cam_frame = new Frame(Pi::player->GetFrame(), "", Frame::TEMP_VIEWING);
	
	if (Pi::GetCamType() == Pi::CAM_FRONT) {
		cam_frame->SetPosition(Pi::player->GetPosition());
	} else if (Pi::GetCamType() == Pi::CAM_REAR) {
		glRotatef(180.0f, 0, 1, 0);
		cam_frame->SetPosition(Pi::player->GetPosition());
	} else /* CAM_EXTERNAL */ {
		cam_frame->SetPosition(Pi::player->GetPosition() + Pi::player->GetExternalViewTranslation());
		Pi::player->ApplyExternalViewRotation();
	}
	Pi::player->ViewingRotation();
	
	glGetDoublev (GL_MODELVIEW_MATRIX, &viewingRotation[0]);

	glCallList(m_bgstarsDlist);
	// position light at sol
	vector3d lpos = Frame::GetFramePosRelativeToOther(Space::GetRootFrame(), cam_frame);
	float lightPos[4];
	lightPos[0] = lpos.x;
	lightPos[1] = lpos.y;
	lightPos[2] = lpos.z;
	lightPos[3] = 0;
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, lightCol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);

	Space::Render(cam_frame);
	Pi::player->DrawHUD(cam_frame);

	Pi::player->GetFrame()->RemoveChild(cam_frame);
	delete cam_frame;
}

void WorldView::Update()
{
	if (Pi::GetSelectedSystem() /* && isn't current system */ ) {
		m_hyperspaceButton->Show();
	} else {
		m_hyperspaceButton->Hide();
	}
	// player control inputs
	Pi::player->AITurn();
}
