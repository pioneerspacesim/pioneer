#include "../libs.h"
#include "GoodsTrader.h"
#include "../Serializer.h"
#include "../Pi.h"
#include "../Player.h"
#include "../ShipCpanel.h"
#include "../NameGenerator.h"
#include "../CommodityTradeWidget.h"

#define DESC_MAX 6
const char *g_business[DESC_MAX] = {
	"Honest %0's Goods Emporium",
	"%0's Trading House",
	"%0's Warehouse",
	"%0's Goods Exchange",
	"%0 Holdings",
	"%0 & Sons"
};
#define BBTEXT_MAX 1
const char *g_bbtext[BBTEXT_MAX] = {
	"GOODS BOUGHT AND SOLD. At %0. Excellent prices paid for rare items."
};

GoodsTrader::GoodsTrader(int type): Mission(type)
{
	SetMoney(10000000);
}

void GoodsTrader::Randomize()
{
	int x = Pi::rng.Int32(DESC_MAX);
	m_priceIncPercent = Pi::rng.Int32(180,220);
	std::string surname = NameGenerator::Surname(Pi::rng);
	m_bbtextidx = Pi::rng.Int32(BBTEXT_MAX);
	m_name = string_subst(g_business[x], 1, &surname);
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		m_equipmentStock[i] = Pi::rng.Int32(50);
	}
}

std::string GoodsTrader::GetBulletinBoardText()
{
	return string_subst(g_bbtext[m_bbtextidx], 1, &m_name);
}

void GoodsTrader::StartChat(MissionChatForm *form)
{
	form->Message(("Welcome to "+m_name).c_str());
	CommodityTradeWidget *commodityTradeWidget = new CommodityTradeWidget(this);
	commodityTradeWidget->onClickBuy.connect(sigc::bind(sigc::mem_fun(this, &GoodsTrader::OnClickBuy), form));
	commodityTradeWidget->onClickSell.connect(sigc::bind(sigc::mem_fun(this, &GoodsTrader::OnClickSell), form));
	form->Add(commodityTradeWidget, 0, 40);
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(form, &MissionChatForm::Close));
	form->Add(b,0,300);
	form->Add(new Gui::Label("Hang up"), 25, 300);
	form->m_optregion->Hide();
}

void GoodsTrader::OnClickBuy(int commodity_type, MissionChatForm *form) {
	SellItemTo(Pi::player, (Equip::Type)commodity_type);
	form->onSomethingChanged.emit();
}
void GoodsTrader::OnClickSell(int commodity_type, MissionChatForm *form) {
	Pi::player->SellItemTo(this, (Equip::Type)commodity_type);
	form->onSomethingChanged.emit();
}

void GoodsTrader::FormResponse(MissionChatForm *form, int resp)
{
	if (resp==0) form->Close();
}

/* MarketAgent shite */
void GoodsTrader::Bought(Equip::Type t) {
	m_equipmentStock[(int)t]++;
}
void GoodsTrader::Sold(Equip::Type t) {
	m_equipmentStock[(int)t]--;
}
bool GoodsTrader::CanBuy(Equip::Type t) const {
	return true;
}
bool GoodsTrader::CanSell(Equip::Type t) const {
	return m_equipmentStock[(int)t] > 0;
}
bool GoodsTrader::DoesSell(Equip::Type t) const {
	return !Polit::IsCommodityLegal(Pi::currentSystem, t);
}
int GoodsTrader::GetPrice(Equip::Type t) const {

	return (m_priceIncPercent * EquipType::types[t].basePrice) / 100;
}

void GoodsTrader::_Load()
{
	using namespace Serializer::Read;
	m_name = rd_string();
	m_bbtextidx = rd_int();
	m_priceIncPercent = rd_int();
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		m_equipmentStock[i] = rd_int();
	}
}

void GoodsTrader::_Save()
{
	using namespace Serializer::Write;
	wr_string(m_name);
	wr_int(m_bbtextidx);
	wr_int(m_priceIncPercent);
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		wr_int(m_equipmentStock[i]);
	}
}

