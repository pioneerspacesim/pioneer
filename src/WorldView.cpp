#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"

static const float lightCol[] = { 1,1,.9,0 };
const float WorldView::PICK_OBJECT_RECT_SIZE = 20.0f;

#define BG_STAR_MAX	2000

WorldView::WorldView(): View()
{
	SetTransparency(true);
	
	Gui::MultiStateImageButton *wheels_button = new Gui::MultiStateImageButton();
	wheels_button->SetShortcut(SDLK_F7, KMOD_NONE);
	wheels_button->AddState(0, "icons/wheels_up.png");
	wheels_button->AddState(1, "icons/wheels_down.png");
	wheels_button->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeWheelsState));
	m_rightButtonBar->Add(wheels_button, 34, 2);

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

void WorldView::OnChangeWheelsState(Gui::MultiStateImageButton *b)
{
	Pi::player->SetWheelState(b->GetState());
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
	float fracH = Pi::GetScrHeight() * 0.5f / Pi::GetScrWidth();
	glFrustum(-0.5, 0.5, -fracH, fracH, 1.0f, 1000.0f);
//	glDepthRange (-10, -100000);		// JJ: uh, what
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

void WorldView::OnMouseDown(Gui::MouseButtonEvent *e)
{
	if(1 == e->button) {
		// Left click in view => Select target.
		float screenPos[2];
		GetPosition(screenPos);
		// Put mouse coords into screen space.
		screenPos[0] += e->x;
		screenPos[1] += e->y;
		Pi::player->SetTarget(PickBody(screenPos[0], screenPos[1]));
	}
}

Body* WorldView::PickBody(const float screenX, const float screenY) const
{
	Body *selected = 0;

	for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		Body *b = *i;
		if(b->IsOnscreen()) {
			const vector3d& _pos = b->GetProjectedPos();
			const float x1 = _pos.x - PICK_OBJECT_RECT_SIZE * 0.5f;
			const float x2 = x1 + PICK_OBJECT_RECT_SIZE;
			const float y1 = _pos.y - PICK_OBJECT_RECT_SIZE * 0.5f;
			const float y2 = y1 + PICK_OBJECT_RECT_SIZE;
			if(screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2) {
				selected = b;
				break;
			}			
		}
	}

	return selected;
}
