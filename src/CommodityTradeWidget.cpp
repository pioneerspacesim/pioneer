#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "CommodityTradeWidget.h"
#include "Lang.h"

#define RBUTTON_DELAY 500
#define RBUTTON_REPEAT 50

struct icon_map_t {
	Equip::Type type;
	std::string icon;
};

static const icon_map_t icon_names[] = {
	{ Equip::HYDROGEN,              "Hydrogen"              },
	{ Equip::LIQUID_OXYGEN,         "Liquid_Oxygen"         },
	{ Equip::METAL_ORE,             "Metal_ore"             },
	{ Equip::CARBON_ORE,            "Carbon_ore"            },
	{ Equip::METAL_ALLOYS,          "Metal_alloys"          },
	{ Equip::PLASTICS,              "Plastics"              },
	{ Equip::FRUIT_AND_VEG,         "Fruit_and_Veg"         },
	{ Equip::ANIMAL_MEAT,           "Animal_Meat"           },
	{ Equip::LIVE_ANIMALS,          "Live_Animals"          },
	{ Equip::LIQUOR,                "Liquor"                },
	{ Equip::GRAIN,                 "Grain"                 },
	{ Equip::TEXTILES,              "Textiles"              },
	{ Equip::FERTILIZER,            "Fertilizer"            },
	{ Equip::WATER,                 "Water"                 },
	{ Equip::MEDICINES,             "Medicines"             },
	{ Equip::CONSUMER_GOODS,        "Consumer_goods"        },
	{ Equip::COMPUTERS,             "Computers"             },
	{ Equip::ROBOTS,                "Robots"                },
	{ Equip::PRECIOUS_METALS,       "Precious_metals"       },
	{ Equip::INDUSTRIAL_MACHINERY,  "Industrial_machinery"  },
	{ Equip::FARM_MACHINERY,        "Farm_machinery"        },
	{ Equip::MINING_MACHINERY,      "Mining_machinery"      },
	{ Equip::AIR_PROCESSORS,        "Air_processors"        },
	{ Equip::SLAVES,                "Slaves"                },
	{ Equip::HAND_WEAPONS,          "Hand_weapons"          },
	{ Equip::BATTLE_WEAPONS,        "Battle_weapons"        },
	{ Equip::NERVE_GAS,             "Nerve_Gas"             },
	{ Equip::NARCOTICS,             "Narcotics"             },
	{ Equip::MILITARY_FUEL,         "Military_fuel"         },
	{ Equip::RUBBISH,               "Rubbish"               },
	{ Equip::RADIOACTIVES,          "Radioactive_waste"     },
    { Equip::NONE, "" }
};

static std::map<Equip::Type,std::string> s_iconMap;

CommodityTradeWidget::CommodityTradeWidget(MarketAgent *seller): Gui::VBox()
{
	SetTransparency(false);
	m_seller = seller;

	if (!s_iconMap.size())
		for (const icon_map_t *scan = icon_names; scan->type != Equip::NONE; scan++)
			s_iconMap.insert(std::make_pair(scan->type, scan->icon));
}

void CommodityTradeWidget::ShowAll()
{
	DeleteAllChildren();
	m_stockLabels.clear();
	m_cargoLabels.clear();

	SetTransparency(true);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 2.5f);
	for (int i=Equip::FIRST_COMMODITY; i<=Equip::LAST_COMMODITY; i++) {
		assert(Equip::types[i].slot == Equip::SLOT_CARGO);

		if (m_seller->DoesSell(Equip::Type(i))) {
				NUM_ITEMS++;
		}
	}
	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	innerbox->SetTransparency(true);
	
	const float iconOffset = 8.0f;
	for (int i=Equip::FIRST_COMMODITY, num=0; i<=Equip::LAST_COMMODITY; i++) {
		assert(Equip::types[i].slot == Equip::SLOT_CARGO);

		if (!m_seller->DoesSell(Equip::Type(i))) continue;
		int stock = m_seller->GetStock(static_cast<Equip::Type>(i));

        std::map<Equip::Type,std::string>::iterator icon_iter = s_iconMap.find(Equip::Type(i));
		if (icon_iter != s_iconMap.end()) {
			Gui::Image *icon = new Gui::Image(("icons/goods/" + (*icon_iter).second + ".png").c_str());
			innerbox->Add(icon, 0, num*YSEP);
		}

		Gui::Label *l = new Gui::Label(Equip::types[i].name);
		if (Equip::types[i].description)
			l->SetToolTip(Equip::types[i].description);
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
		
		snprintf(buf, sizeof(buf), "%dt", stock*Equip::types[i].mass);
		Gui::Label *stocklabel = new Gui::Label(buf);
		m_stockLabels[i] = stocklabel;
		innerbox->Add(stocklabel, 275, num*YSEP+iconOffset);
		
		snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i))*Equip::types[i].mass);
		Gui::Label *cargolabel = new Gui::Label(buf);
		m_cargoLabels[i] = cargolabel;
		innerbox->Add(cargolabel, 325, num*YSEP+iconOffset);
		num++;
	}
	innerbox->ShowAll();

	portal->Add(innerbox);
	portal->ShowAll();

	Gui::Fixed *heading = new Gui::Fixed(470, Gui::Screen::GetFontHeight());
	const float *col = Gui::Theme::Colors::tableHeading;
	heading->Add((new Gui::Label(Lang::ITEM))->Color(col), 0, 0);
	heading->Add((new Gui::Label(Lang::PRICE))->Color(col), 200, 0);
	heading->Add((new Gui::Label(Lang::BUY))->Color(col), 380, 0);
	heading->Add((new Gui::Label(Lang::SELL))->Color(col), 415, 0);
	heading->Add((new Gui::Label(Lang::STOCK))->Color(col), 275, 0);
	heading->Add((new Gui::Label(Lang::CARGO))->Color(col), 325, 0);
	PackEnd(heading);

	Gui::HBox *body = new Gui::HBox();
	body->PackEnd(portal);
	body->PackEnd(scroll);
	PackEnd(body);

	SetSpacing(YSEP-Gui::Screen::GetFontHeight());

	Gui::VBox::ShowAll();
}

void CommodityTradeWidget::UpdateStock(int commodity_type)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(commodity_type))*Equip::types[commodity_type].mass);
	m_cargoLabels[commodity_type]->SetText(buf);
	
	snprintf(buf, sizeof(buf), "%dt", m_seller->GetStock(static_cast<Equip::Type>(commodity_type))*Equip::types[commodity_type].mass);
	m_stockLabels[commodity_type]->SetText(buf);
}

