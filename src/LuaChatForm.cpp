#include "Pi.h"
#include "Player.h"
#include "LuaChatForm.h"
#include "libs.h"
#include "Gui.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "PoliceChatForm.h"
#include "PiLuaModules.h"
#include "CommodityTradeWidget.h"

EXPORT_OOLUA_FUNCTIONS_10_NON_CONST(LuaChatForm,
		UpdateBaseDisplay,
		Close,
		Clear,
		GotoPolice,
		SetTitle,
		SetMessage,
		AddOption,
		AddTraderWidget,
		RemoveAdvertOnClose,
		SetStage)
EXPORT_OOLUA_FUNCTIONS_2_CONST(LuaChatForm,
		GetStage,
		GetAdRef)

LuaChatForm::~LuaChatForm()
{
	if (m_adTaken) m_station->BBRemoveAdvert(m_modName, m_modRef);
}

void LuaChatForm::OnAdvertDeleted()
{
	m_advert = 0;
	Clear();
	SetMessage("Sorry, this advert has expired.");
	AddOption("Hang up.", 0);
}

void LuaChatForm::StartChat(SpaceStation *s, const BBAdvert *a)
{
	m_adTaken = false;
	m_advert = a;
	m_station = s;
	m_modName = a->GetModule();
	m_modRef = a->GetLuaRef();
	m_stage = "";
	CallDialogHandler(0);
}

void LuaChatForm::AddTraderWidget()
{
	CommodityTradeWidget *w = new CommodityTradeWidget(this);
	w->onClickBuy.connect(sigc::mem_fun(this, &LuaChatForm::OnClickBuy));
	w->onClickSell.connect(sigc::mem_fun(this, &LuaChatForm::OnClickSell));
	Gui::Fixed *f = new Gui::Fixed(400.0, 200.0);
	f->Add(w, 0, 0);
	m_msgregion->PackEnd(f);

	m_commodityTradeWidget = w;
}

void LuaChatForm::AddOption(const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &LuaChatForm::CallDialogHandler), val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}

void LuaChatForm::CallDialogHandler(int optionClicked)
{
	if (m_advert == 0) {
		// advert has expired
		Close();
	} else {
		PiLuaModules::ModCall(m_modName.c_str(), "onChatBB", 0, this, optionClicked);
	}
}

Sint64 LuaChatForm::GetPrice(Equip::Type t) const {
	lua_State *l = PiLuaModules::GetLuaState();
	PiLuaModules::ModCall(m_modName.c_str(), "TraderGetPrice", 1, this, (int)t);
	Sint64 price = (Sint64)(100.0*lua_tonumber(l, -1));
	lua_pop(l, 1);
	return price;
}

bool LuaChatForm::CanBuy(Equip::Type t, bool verbose) const {
	return DoesSell(t);
}
bool LuaChatForm::CanSell(Equip::Type t, bool verbose) const {
	return (GetStock(t) > 0) && DoesSell(t);
}
bool LuaChatForm::DoesSell(Equip::Type t) const {
	lua_State *l = PiLuaModules::GetLuaState();
	PiLuaModules::ModCall(m_modName.c_str(), "TraderCanTrade", 1, this, (int)t);
	bool can = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);
	return can;
}
int LuaChatForm::GetStock(Equip::Type t) const {
	lua_State *l = PiLuaModules::GetLuaState();
	PiLuaModules::ModCall(m_modName.c_str(), "TraderGetStock", 1, this, (int)t);
	int stock = lua_tointeger(l, -1);
	lua_pop(l, 1);
	return stock;
}
void LuaChatForm::Bought(Equip::Type t) {
	PiLuaModules::ModCall(m_modName.c_str(), "TraderBought", 0, this, (int)t);
}
void LuaChatForm::Sold(Equip::Type t) {
	PiLuaModules::ModCall(m_modName.c_str(), "TraderSold", 0, this, (int)t);
}

void LuaChatForm::OnClickBuy(int equipType) {
	lua_State *l = PiLuaModules::GetLuaState();
	PiLuaModules::ModCall(m_modName.c_str(), "TraderOnClickBuy", 1, this, equipType);

	bool doBuy = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);
	if (doBuy) {
		if (SellTo(Pi::player, (Equip::Type)equipType, true)) {
			Pi::Message(stringf(512, "You have bought 1t of %s.", EquipType::types[equipType].name));
		}
		m_commodityTradeWidget->UpdateStock(equipType);
		UpdateBaseDisplay();
	}
}
void LuaChatForm::OnClickSell(int equipType) {
	lua_State *l = PiLuaModules::GetLuaState();

	PiLuaModules::ModCall(m_modName.c_str(), "TraderOnClickSell", 1, this, equipType);
	bool doSell = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);
	if (doSell) {
		if (BuyFrom(Pi::player, (Equip::Type)equipType, true)) {
			Pi::Message(stringf(512, "You have sold 1t of %s.", EquipType::types[equipType].name));
		}
		m_commodityTradeWidget->UpdateStock(equipType);
		UpdateBaseDisplay();
	}
}

void LuaChatForm::GotoPolice() {
	Close();
	Pi::spaceStationView->JumpTo(new PoliceChatForm());
}
