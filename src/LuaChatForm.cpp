#include "Pi.h"
#include "Player.h"
#include "LuaChatForm.h"
#include "LuaUtils.h"
#include "libs.h"
#include "Gui.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "PoliceChatForm.h"
#include "CommodityTradeWidget.h"
#include "LuaObject.h"
#include "LuaConstants.h"

LuaChatForm::~LuaChatForm()
{
	if (m_adTaken) RemoveAdvert();
}

void LuaChatForm::CallDialogHandler(int optionClicked)
{
	if (GetAdvert() == 0) {
		// advert has expired
		Close();
	} else {
	    SetMoney(1000000000);

		lua_State *l = Pi::luaManager.GetLuaState();

		LUA_DEBUG_START(l);

		lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
		assert(lua_istable(l, -1));

		lua_pushinteger(l, GetAdvert()->ref);
		lua_gettable(l, -2);
		assert(!lua_isnil(l, -1));

		lua_getfield(l, -1, "onChat");
		assert(lua_isfunction(l, -1));

		LuaObject<LuaChatForm>::PushToLua(this);
		lua_pushinteger(l, GetAdvert()->ref);
		lua_pushinteger(l, optionClicked);
		lua_call(l, 3, 0);

		lua_pop(l, 2);

		LUA_DEBUG_END(l, 0);
	}
}

void LuaChatForm::RemoveAdvert() {
	lua_State *l = Pi::luaManager.GetLuaState();

	int ref = GetAdvert()->ref;

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	assert(lua_istable(l, -1));

	lua_pushinteger(l, ref);
	lua_gettable(l, -2);
	assert(!lua_isnil(l, -1));

	lua_getfield(l, -1, "onDelete");
	if (!lua_isnil(l, -1)) {
		lua_pushinteger(l, ref);
		lua_call(l, 1, 0);
	}
	else
		lua_pop(l, 1);

	lua_pop(l, 1);

	lua_pushinteger(l, ref);
	lua_pushnil(l);
	lua_settable(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

    GetStation()->RemoveBBAdvert(ref);
}

static inline void _get_trade_function(lua_State *l, int ref, const char *name)
{
	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	assert(lua_istable(l, -1));

	lua_pushinteger(l, ref);
	lua_gettable(l, -2);
	assert(!lua_isnil(l, -1));

	lua_getfield(l, -1, "tradeWidgetFunctions");
	assert(lua_istable(l, -1));

	lua_getfield(l, -1, name);
	assert(lua_isfunction(l, -1));

	lua_insert(l, -4);
	lua_pop(l, 3);

	LUA_DEBUG_END(l, 1);
}

bool LuaChatForm::CanBuy(Equip::Type t, bool verbose) const {
	return DoesSell(t);
}
bool LuaChatForm::CanSell(Equip::Type t, bool verbose) const {
	return (GetStock(t) > 0) && DoesSell(t);
}
bool LuaChatForm::DoesSell(Equip::Type t) const {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "canTrade");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 1);

	bool can_trade = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return can_trade;
}

int LuaChatForm::GetStock(Equip::Type t) const {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "getStock");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 1);

	int stock = lua_tointeger(l, -1);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return stock;
}

Sint64 LuaChatForm::GetPrice(Equip::Type t) const {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "getPrice");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 1);

	Sint64 price = (Sint64)(lua_tonumber(l, -1) * 100.0);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return price;
}

void LuaChatForm::OnClickBuy(int t) {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "onClickBuy");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 1);

	bool allow_buy = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	if (allow_buy) {
		if (SellTo(Pi::player, static_cast<Equip::Type>(t), true)) {
			Pi::Message(stringf(512, "You have bought 1t of %s.", EquipType::types[t].name));
		}
		m_commodityTradeWidget->UpdateStock(t);
		UpdateBaseDisplay();
	}
}

void LuaChatForm::OnClickSell(int t) {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "onClickSell");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 1);

	bool allow_sell = lua_toboolean(l, -1) != 0;
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	if (allow_sell) {
		if (BuyFrom(Pi::player, static_cast<Equip::Type>(t), true)) {
			Pi::Message(stringf(512, "You have sold 1t of %s.", EquipType::types[t].name));
		}
		m_commodityTradeWidget->UpdateStock(t);
		UpdateBaseDisplay();
	}
}

void LuaChatForm::Bought(Equip::Type t) {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "bought");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 0);

	LUA_DEBUG_END(l, 0);
}

void LuaChatForm::Sold(Equip::Type t) {
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	_get_trade_function(l, GetAdvert()->ref, "sold");

	lua_pushinteger(l, GetAdvert()->ref);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", t));
	lua_call(l, 2, 0);

	LUA_DEBUG_END(l, 0);
}

static int l_luachatform_clear(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	dialog->Clear();
	return 0;
}

static int l_luachatform_set_title(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	std::string title = luaL_checkstring(l, 2);
	dialog->SetTitle(title.c_str());
	return 0;
}

static int l_luachatform_set_message(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	std::string message = luaL_checkstring(l, 2);
	dialog->SetMessage(message.c_str());
	return 0;
}

static int l_luachatform_add_option(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	std::string text = luaL_checkstring(l, 2);
	int val = luaL_checkinteger(l, 3);
	dialog->AddOption(text, val);
	return 0;
}

static inline void _bad_trade_function(lua_State *l, const char *name) {
	luaL_where(l, 0);
	luaL_error(l, "%s bad argument '%s' to 'AddGoodsTrader' (function expected, got %s)", lua_tostring(l, -1), name, luaL_typename(l, -2));
}

static inline void _cleanup_trade_functions(GenericChatForm *form, lua_State *l, int ref)
{
	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	assert(lua_istable(l, -1));

	lua_pushinteger(l, ref);
	lua_gettable(l, -2);
	assert(!lua_isnil(l, -1));

	lua_pushstring(l, "tradeWidgetFunctions");
	lua_pushnil(l);
	lua_settable(l, -3);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

int LuaChatForm::l_luachatform_add_goods_trader(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);

	if(!lua_istable(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TTABLE));
	
	// XXX verbose but what can you do?
	lua_getfield(l, 2, "canTrade");
	if (!lua_isfunction(l, -1))
		_bad_trade_function(l, "canTrade");

	lua_getfield(l, 2, "getStock");
	if (!lua_isfunction(l, -1))
		_bad_trade_function(l, "getStock");

	lua_getfield(l, 2, "getPrice");
	if (!lua_isfunction(l, -1))
		_bad_trade_function(l, "getPrice");

	lua_getfield(l, 2, "onClickBuy");
	if(!lua_isfunction(l, -1) && !lua_isnil(l, -1))
		_bad_trade_function(l, "onClickBuy");

	lua_getfield(l, 2, "onClickSell");
	if(!lua_isfunction(l, -1) && !lua_isnil(l, -1))
		_bad_trade_function(l, "onClickSell");

	lua_getfield(l, 2, "bought");
	if(!lua_isfunction(l, -1) && !lua_isnil(l, -1))
		_bad_trade_function(l, "bought");

	lua_getfield(l, 2, "sold");
	if(!lua_isfunction(l, -1) && !lua_isnil(l, -1))
		_bad_trade_function(l, "sold");

	lua_pop(l, 6);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	assert(lua_istable(l, -1));

	lua_pushinteger(l, dialog->GetAdvert()->ref);
	lua_gettable(l, -2);
	assert(!lua_isnil(l, -1));

	lua_pushstring(l, "tradeWidgetFunctions");
	lua_pushvalue(l, 2);
	lua_settable(l, -3);

	lua_pop(l, 2);

	CommodityTradeWidget *w = new CommodityTradeWidget(dialog);
	w->onClickBuy.connect(sigc::mem_fun(dialog, &LuaChatForm::OnClickBuy));
	w->onClickSell.connect(sigc::mem_fun(dialog, &LuaChatForm::OnClickSell));
	Gui::Fixed *f = new Gui::Fixed(400.0, 200.0);
	f->Add(w, 0, 0);
	dialog->m_msgregion->PackEnd(f);

	dialog->m_commodityTradeWidget = w;

	dialog->onClose.connect(sigc::bind(sigc::ptr_fun(_cleanup_trade_functions), l, dialog->GetAdvert()->ref));

	return 0;
}

static int l_luachatform_close(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	dialog->Close();
	return 0;
}

static int l_luachatform_refresh(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	dialog->UpdateBaseDisplay();
	return 0;
}

static int l_luachatform_goto_police(lua_State *l)
{
	Pi::spaceStationView->JumpToForm(new PoliceChatForm());
	return 0;
}

static int l_luachatform_remove_advert_on_close(lua_State *l)
{
	LuaChatForm *dialog = LuaObject<LuaChatForm>::GetFromLua(1);
	dialog->RemoveAdvertOnClose();
	return 0;
}

template <> const char *LuaObject<LuaChatForm>::s_type = "ChatForm";

template <> void LuaObject<LuaChatForm>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "Clear",               l_luachatform_clear                         },
		{ "SetTitle",            l_luachatform_set_title                     },
		{ "SetMessage",          l_luachatform_set_message                   },
		{ "AddOption",           l_luachatform_add_option                    },
		{ "AddGoodsTrader",      LuaChatForm::l_luachatform_add_goods_trader },
		{ "Close",               l_luachatform_close                         },
		{ "Refresh",             l_luachatform_refresh                       },
		{ "GotoPolice",          l_luachatform_goto_police                   },
		{ "RemoveAdvertOnClose", l_luachatform_remove_advert_on_close        },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
