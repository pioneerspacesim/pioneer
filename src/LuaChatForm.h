// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUACHATFORM_H
#define _LUACHATFORM_H

#include "StationAdvertForm.h"
#include "MarketAgent.h"
#include "DeleteEmitter.h"
#include "LuaManager.h"

struct BBAdvert;
class CommodityTradeWidget;

class LuaChatForm: public StationAdvertForm, public MarketAgent, public DeleteEmitter {
	friend class LuaObject<LuaChatForm>;

public:
	LuaChatForm(FormController *controller, SpaceStation *station, const BBAdvert &ad) :
		StationAdvertForm(controller, station, ad), m_commodityTradeWidget(0) {}

	virtual void OnOptionClicked(int option);
	virtual void OnClose();

	/* MarketAgent stuff */
	virtual Sint64 GetPrice(Equip::Type t) const;
	virtual bool CanBuy(Equip::Type t, bool verbose) const;
	virtual bool CanSell(Equip::Type t, bool verbose) const;
	virtual bool DoesSell(Equip::Type t) const;
	virtual int GetStock(Equip::Type t) const;

protected:
	/* MarketAgent stuff */
	virtual void Bought(Equip::Type t);
	virtual void Sold(Equip::Type t);

private:
	CommodityTradeWidget *m_commodityTradeWidget;
	void OnClickBuy(int equipType);
	void OnClickSell(int equipType);

	static int l_luachatform_set_message(lua_State *l);
	static int l_luachatform_add_option(lua_State *l);
	static int l_luachatform_clear(lua_State *l);
	static int l_luachatform_close(lua_State *l);
	static int l_luachatform_add_goods_trader(lua_State *l);
	static int l_luachatform_goto_police(lua_State *l);
};

#endif /* _LUACHATFORM_H */
