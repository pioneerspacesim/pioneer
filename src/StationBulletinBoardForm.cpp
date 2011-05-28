#include "StationBulletinBoardForm.h"
#include "SpaceStation.h"
#include "Pi.h"
#include "Player.h"

StationBulletinBoardForm::StationBulletinBoardForm() : FaceForm()
{
	SpaceStation *station = Pi::player->GetDockedWith();

	SetTitle(station->GetLabel() + " bulletin board");

	Gui::VScrollPortal *scrollbox = new Gui::VScrollPortal(460);
	Gui::VScrollBar *scrollbar = new Gui::VScrollBar();
	scrollbar->SetAdjustment(&scrollbox->vscrollAdjust);

	const std::list<const BBAdvert*> adverts = station->GetBBAdverts();

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 5);

	Gui::Fixed *advertbox = new Gui::Fixed(450, adverts.size() * YSEP);
	int y = 0;
	for (std::list<const BBAdvert*>::const_iterator i = adverts.begin(); i != adverts.end(); i++) {
		Gui::SolidButton *b = new Gui::SolidButton();
		advertbox->Add(b, 0, y);

		Gui::Label *l = new Gui::Label((*i)->description);
		advertbox->Add(l, 20, y);

		y += YSEP;
	}
	advertbox->ShowAll();

	scrollbox->Add(advertbox);
	scrollbox->ShowAll();

	Add(scrollbox, 0, 0);
	Add(scrollbar, 460, 10);

	ShowAll();
}
