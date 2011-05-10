#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "CommodityTradeWidget.h"

#define RBUTTON_DELAY 500
#define RBUTTON_REPEAT 50

CommodityTradeWidget::CommodityTradeWidget(MarketAgent *seller): Gui::Fixed()
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
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 2.5f);
	for (int i=(int)Equip::FIRST_COMMODITY; i<=Equip::LAST_COMMODITY; i++) {
		assert(EquipType::types[i].slot == Equip::SLOT_CARGO);

		if (m_seller->DoesSell((Equip::Type)i)) {
				NUM_ITEMS++;
		}
	}
	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	innerbox->SetTransparency(true);
	
	const float iconOffset = 8.0f;
	for (int i=(int)Equip::FIRST_COMMODITY, num=0; i<=Equip::LAST_COMMODITY; i++) {
		assert(EquipType::types[i].slot == Equip::SLOT_CARGO);

		if (!m_seller->DoesSell((Equip::Type)i)) continue;
		int stock = m_seller->GetStock(static_cast<Equip::Type>(i));

		// need to replace spaces in the item name
		std::string imgname = std::string(EquipType::types[i].name);
		size_t imgbad;
		while ((imgbad = imgname.find(' ')) != std::string::npos) {
			imgname.replace(imgbad, 1, "_");
		}
		Gui::Image *img = new Gui::Image((PIONEER_DATA_DIR "/icons/goods/" + imgname + ".png").c_str() );

		innerbox->Add(img,0, num*YSEP);
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		if (EquipType::types[i].description)
			l->SetToolTip(EquipType::types[i].description);
		innerbox->Add(l,42,num*YSEP+iconOffset);
		Gui::Button *b = new Gui::RepeaterButton(RBUTTON_DELAY, RBUTTON_REPEAT);
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CommodityTradeWidget::OnClickBuy), i));
		innerbox->Add(b, 380, num*YSEP+iconOffset);
		b = new Gui::RepeaterButton(RBUTTON_DELAY, RBUTTON_REPEAT);
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CommodityTradeWidget::OnClickSell), i));
		innerbox->Add(b, 415, num*YSEP+iconOffset);
		char buf[128];
		innerbox->Add(new Gui::Label(
					format_money(m_seller->GetPrice(static_cast<Equip::Type>(i)))
					), 200, num*YSEP+iconOffset);
		
		snprintf(buf, sizeof(buf), "%dt", stock*EquipType::types[i].mass);
		Gui::Label *stocklabel = new Gui::Label(buf);
		m_stockLabels[i] = stocklabel;
		innerbox->Add(stocklabel, 275, num*YSEP+iconOffset);
		
		snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i))*EquipType::types[i].mass);
		Gui::Label *cargolabel = new Gui::Label(buf);
		m_cargoLabels[i] = cargolabel;
		innerbox->Add(cargolabel, 325, num*YSEP+iconOffset);
		num++;
	}
	innerbox->ShowAll();

	const float *col = Gui::Theme::Colors::tableHeading;
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

