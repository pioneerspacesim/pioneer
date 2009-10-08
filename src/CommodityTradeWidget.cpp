#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "CommodityTradeWidget.h"

#define RBUTTON_DELAY 500
#define RBUTTON_REPEAT 50

CommodityTradeWidget::CommodityTradeWidget(MarketAgent *seller): Gui::Fixed(470, 400)
{
	SetTransparency(false);
	m_seller = seller;
}

void CommodityTradeWidget::ShowAll()
{
	DeleteAllChildren();
	m_stockLabels.clear();
	m_cargoLabels.clear();

	SetTransparency(true);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,400);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((EquipType::types[i].slot == Equip::SLOT_CARGO) &&
		    (m_seller->DoesSell((Equip::Type)i))) {
				NUM_ITEMS++;
		}
	}
	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	innerbox->SetTransparency(true);
	
	for (int i=1, num=0; i<Equip::TYPE_MAX; i++) {
		if (EquipType::types[i].slot != Equip::SLOT_CARGO) continue;
		if (!m_seller->DoesSell((Equip::Type)i)) continue;
		int stock = m_seller->GetStock(static_cast<Equip::Type>(i));
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		if (EquipType::types[i].description)
			l->SetToolTip(EquipType::types[i].description);
		innerbox->Add(l,0,num*YSEP);
		Gui::Button *b = new Gui::RepeaterButton(RBUTTON_DELAY, RBUTTON_REPEAT);
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CommodityTradeWidget::OnClickBuy), i));
		innerbox->Add(b, 380, num*YSEP);
		b = new Gui::RepeaterButton(RBUTTON_DELAY, RBUTTON_REPEAT);
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CommodityTradeWidget::OnClickSell), i));
		innerbox->Add(b, 415, num*YSEP);
		char buf[128];
		innerbox->Add(new Gui::Label(
					format_money(m_seller->GetPrice(static_cast<Equip::Type>(i)))
					), 200, num*YSEP);
		
		snprintf(buf, sizeof(buf), "%dt", stock*EquipType::types[i].mass);
		Gui::Label *stocklabel = new Gui::Label(buf);
		m_stockLabels[i] = stocklabel;
		innerbox->Add(stocklabel, 275, num*YSEP);
		
		snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i))*EquipType::types[i].mass);
		Gui::Label *cargolabel = new Gui::Label(buf);
		m_cargoLabels[i] = cargolabel;
		innerbox->Add(cargolabel, 325, num*YSEP);
		num++;
	}
	innerbox->ShowAll();

	const float *col = Gui::Color::tableHeading;
	Add((new Gui::Label("Item"))->Color(col), 0, 0);
	Add((new Gui::Label("Price"))->Color(col), 200, 0);
	Add((new Gui::Label("Buy"))->Color(col), 380, 0);
	Add((new Gui::Label("Sell"))->Color(col), 415, 0);
	Add((new Gui::Label("Stock"))->Color(col), 275, 0);
	Add((new Gui::Label("Cargo"))->Color(col), 325, 0);
	Add(portal, 0, YSEP);
	Add(scroll, 455, YSEP);
	portal->Add(innerbox);
	portal->ShowAll();

	Gui::Fixed::ShowAll();
}

void CommodityTradeWidget::UpdateStock(int commodity_type)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(commodity_type))*EquipType::types[commodity_type].mass);
	m_cargoLabels[commodity_type]->SetText(buf);
	
	snprintf(buf, sizeof(buf), "%dt", m_seller->GetStock(static_cast<Equip::Type>(commodity_type))*EquipType::types[commodity_type].mass);
	m_stockLabels[commodity_type]->SetText(buf);
}

