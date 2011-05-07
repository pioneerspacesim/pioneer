#ifndef _LUACHATFORM_H
#define _LUACHATFORM_H

#include "BBAdvertChatForm.h"
#include "MarketAgent.h"
#include "DeleteEmitter.h"
#include "LuaManager.h"

class BBAdvert;
class CommodityTradeWidget;

class LuaChatForm: public BBAdvertChatForm, public MarketAgent, public DeleteEmitter {
	friend class LuaObject<LuaChatForm>;

public:
    LuaChatForm(SpaceStation *station, const BBAdvert *ad) : BBAdvertChatForm(station, ad), m_adTaken(false) {}
    virtual ~LuaChatForm();

	virtual void CallDialogHandler(int optionClicked);
	virtual void RemoveAdvert();
	void RemoveAdvertOnClose() { m_adTaken = true; }

	/* MarketAgent stuff */
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const;
	int GetStock(Equip::Type t) const;

protected:
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);

private:
	CommodityTradeWidget *m_commodityTradeWidget;
	void OnClickBuy(int equipType);
	void OnClickSell(int equipType);

	static int l_luachatform_add_goods_trader(lua_State *l);

	bool m_adTaken;
};

#endif /* _LUACHATFORM_H */
