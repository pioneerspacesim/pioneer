#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"

SpaceStationView::SpaceStationView(): View()
{
	SetTransparency(false);

	Gui::Label *l = new Gui::Label("Hello friend! Thankyou for docking with this space station!\n"
	"You may have noticed that the docking procedure was not entirely\n"
	"physically correct. This is a result of unimplemented physics in this\n"
	"region of the galaxy. We hope to have things back to normal within a\n"
	"few weeks, and in the mean time would like to offer our apologies for\n"
	"any loss of earnings, immersion or lunch.\n\n"
	"Currently the usual space station services are not available, but we\n"
	"can offer you this promotional message from one of the station's sponsors:\n\n"
	"                       DIET STEAKETTE: IT'S BAD");

	float size[2];
	GetSize(size);
	Add(l, 40, size[1]-100);

	Gui::SolidButton *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SpaceStationView::OnClickRequestLaunch));
	Add(b, 40, size[1]-300);
	l = new Gui::Label("Request Launch");
	Add(l, 65, size[1]-300);


	l = new Gui::Label("Comms Link");
	l->SetColor(1,.7,0);
	m_rightRegion2->Add(l, 10, 3);
}

void SpaceStationView::OnClickRequestLaunch()
{
	printf("Launching!\n");
	Pi::player->SetDockedWith(0);
	Pi::SetView(Pi::world_view);
}

void SpaceStationView::Draw3D()
{
}

void SpaceStationView::Update()
{
}
