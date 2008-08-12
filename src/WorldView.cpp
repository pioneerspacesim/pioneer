#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"
#include "SpaceStation.h"
#include "ShipCpanel.h"

static const float lightCol[] = { 1,1,.9,0 };
const float WorldView::PICK_OBJECT_RECT_SIZE = 20.0f;

#define BG_STAR_MAX	5000

WorldView::WorldView(): View()
{
	float size[2];
	GetSize(size);
	
	SetTransparency(true);
	
	commsOptions = new Fixed(size[0], size[1]/2);
	commsOptions->SetTransparency(true);
	Add(commsOptions, 10, 20);

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
		float col = 0.05+Pi::rng.NDouble(4);
		col = CLAMP(col, 0, 1);
		glColor3f(col, col, col);
		glVertex3f(1000-Pi::rng.Double(2000.0), 1000-Pi::rng.Double(2000.0), 1000-Pi::rng.Double(2000.0));
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
	float fracH = Pi::GetScrHeight() / (float)Pi::GetScrWidth();
	glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(Pi::player) {
		// make temporary camera frame at player
		Frame cam_frame(Pi::player->GetFrame(), "", Frame::TEMP_VIEWING);
		
		if (Pi::GetCamType() == Pi::CAM_FRONT) {
			cam_frame.SetPosition(Pi::player->GetPosition());
		} else if (Pi::GetCamType() == Pi::CAM_REAR) {
			glRotatef(180.0f, 0, 1, 0);
			cam_frame.SetPosition(Pi::player->GetPosition());
		} else /* CAM_EXTERNAL */ {
			cam_frame.SetPosition(Pi::player->GetPosition() + Pi::player->GetExternalViewTranslation());
			Pi::player->ApplyExternalViewRotation();
		}
		Pi::player->ViewingRotation();
		
		glGetDoublev (GL_MODELVIEW_MATRIX, &viewingRotation[0]);

		glCallList(m_bgstarsDlist);
		// position light at sol
		vector3d lpos = vector3d::Normalize(Frame::GetFramePosRelativeToOther(Space::GetRootFrame(), &cam_frame));
		float lightPos[4];
		lightPos[0] = lpos.x;
		lightPos[1] = lpos.y;
		lightPos[2] = lpos.z;
		lightPos[3] = 0;
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, lightCol);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);

		Space::Render(&cam_frame);
		Pi::player->DrawHUD(&cam_frame);

		Pi::player->GetFrame()->RemoveChild(&cam_frame);
	}
}

void WorldView::Update()
{
	if (Pi::GetSelectedSystem() && !Pi::player->GetDockedWith()/* && isn't current system */ ) {
		m_hyperspaceButton->Show();
	} else {
		m_hyperspaceButton->Hide();
	}
	// player control inputs
	if(Pi::player)
		Pi::player->PollControls();

	Body *target = Pi::player->GetNavTarget();
	if (target) {
		commsOptions->ShowAll();
	} else {
		//commsOptions->HideAll();
	}
}

Gui::Button *WorldView::AddCommsOption(std::string msg, int ypos)
{
	Gui::Label *l = new Gui::Label(msg);
	commsOptions->Add(l, 50, ypos);

	Gui::TransparentButton *b = new Gui::TransparentButton();
	commsOptions->Add(b, 16, ypos);
	return b;
}

static void PlayerRequestDockingClearance(SpaceStation *s)
{
	s->GetDockingClearance(Pi::player);
	Pi::cpan->SetTemporaryMessage(s, "Docking clearance granted.");
}

void WorldView::UpdateCommsOptions()
{
	Body * const navtarget = Pi::player->GetNavTarget();
	commsOptions->DeleteAllChildren();
	
	float size[2];
	commsOptions->GetSize(size);
	int ypos = size[1]-16;
	if (navtarget) {
		if (navtarget->GetType() == Object::SPACESTATION) {
			commsOptions->Add(new Gui::Label(navtarget->GetLabel()), 16, ypos);
			ypos -= 32;
			Gui::Button *b = AddCommsOption("Request docking clearance", ypos);
			b->onClick.connect(sigc::bind(sigc::ptr_fun(&PlayerRequestDockingClearance), (SpaceStation*)navtarget));
			ypos -= 32;
		} else {
			commsOptions->Add(new Gui::Label(navtarget->GetLabel()), 16, ypos);
			ypos -= 32;
			std::string msg = "Do something to "+navtarget->GetLabel();
			Gui::Button *b = AddCommsOption(msg, ypos);
			ypos -= 32;
		}
	}
}

bool WorldView::OnMouseDown(Gui::MouseButtonEvent *e)
{
	// if continuing to propagate mouse event, see if target is clicked on
	if (Container::OnMouseDown(e)) {
		if(1 == e->button && !Pi::MouseButtonState(3)) {
			// Left click in view when RMB not pressed => Select target.
			Body* const target = PickBody(e->screenX, e->screenY);
			if(Pi::player) {
				//TODO: if in nav mode, SetNavTarget(), else SetCombatTarget().
				Pi::player->SetNavTarget(target);
			}
		}
	}
	return true;
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
