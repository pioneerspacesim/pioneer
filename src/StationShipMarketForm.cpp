// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StationShipMarketForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "StationShipViewForm.h"
#include "Lang.h"
#include "StringF.h"

StationShipMarketForm::StationShipMarketForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(Lang::SOMEWHERE_SHIP_MARKET, formatarg("station", m_station->GetLabel())));

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450);
	scroll->SetAdjustment(&portal->vscrollAdjust);

	float line_height = Gui::Screen::GetFontHeight();

	m_shiplistBox = new Gui::VBox();
	m_shiplistBox->SetSpacing(line_height);
	UpdateShipList();
	m_shiplistBox->ShowAll();

	portal->Add(m_shiplistBox);
	portal->ShowAll();

	Gui::VBox *outerbox = new Gui::VBox();
	outerbox->SetSpacing(line_height);

	Gui::Fixed *heading = new Gui::Fixed(470, Gui::Screen::GetFontHeight());
	const Color &col = Gui::Theme::Colors::tableHeading;
	heading->Add((new Gui::Label(Lang::SHIP))->Color(col), 0, 0);
	heading->Add((new Gui::Label(Lang::PRICE))->Color(col), 200, 0);
	heading->Add((new Gui::Label(Lang::PART_EX))->Color(col), 275, 0);
	heading->Add((new Gui::Label(Lang::CAPACITY))->Color(col), 370, 0);
	heading->Add((new Gui::Label(Lang::VIEW))->Color(col), 430, 0);
	outerbox->PackEnd(heading);

	Gui::HBox *body = new Gui::HBox();
	body->PackEnd(portal);
	body->PackEnd(scroll);
	outerbox->PackEnd(body);

	Add(outerbox, 0, 0);
	ShowAll();

	m_onShipsForSaleChangedConnection = m_station->onShipsForSaleChanged.connect(sigc::mem_fun(this, &StationShipMarketForm::OnShipsForSaleChanged));
}

StationShipMarketForm::~StationShipMarketForm()
{
	m_onShipsForSaleChangedConnection.disconnect();
}

void StationShipMarketForm::OnShipsForSaleChanged()
{
	UpdateShipList();
	m_shiplistBox->ShowAll();
}

void StationShipMarketForm::UpdateShipList()
{
	m_shiplistBox->DeleteAllChildren();

	float line_height = Gui::Screen::GetFontHeight();

	const std::vector<ShipFlavour> &ships = m_station->GetShipsOnSale();

	int num = 0;
	for (std::vector<ShipFlavour>::const_iterator i = ships.begin(); i!=ships.end(); ++i) {
		Gui::Fixed *f = new Gui::Fixed(450, line_height);

		Gui::Label *l = new Gui::Label(ShipType::types[(*i).id].name);
		f->Add(l,0,0);
		f->Add(new Gui::Label(format_money((*i).price)), 200, 0);
		f->Add(new Gui::Label(format_money((*i).price - Pi::player->GetFlavour()->price) ), 275, 0);
		f->Add(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", ShipType::types[(*i).id].capacity))), 370, 0);

		Gui::SolidButton *sb = new Gui::SolidButton();
		sb->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipMarketForm::ViewShip), num));
		f->Add(sb, 430, 0);

		m_shiplistBox->PackEnd(f);

		num++;
	}
}

void StationShipMarketForm::ViewShip(int num)
{
	m_formController->ActivateForm(new StationShipViewForm(m_formController, num));
}
