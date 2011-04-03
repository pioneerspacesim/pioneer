#ifndef _LUACHATFORM_H
#define _LUACHATFORM_H

#include "GenericChatForm.h"
#include "MarketAgent.h"
#include "DeleteEmitter.h"
#include "LuaManager.h"

class BBAdvert;
class CommodityTradeWidget;

class LuaChatForm: public GenericChatForm, public MarketAgent, public DeleteEmitter {
	friend class LuaObject<LuaChatForm>;

public:
	virtual ~LuaChatForm();
	void AddOption(std::string text, int val);
	void AddTraderWidget();
	void StartChat(SpaceStation *s, const BBAdvert *);
	void CallDialogHandler(int optionClicked);
	void RemoveAdvert();
	/* MarketAgent stuff */
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const;
	int GetStock(Equip::Type t) const;
	void RemoveAdvertOnClose() { m_adTaken = true; }
	void OnAdvertDeleted();
	void GotoPolice();
	const BBAdvert *GetAdvert() const { return m_advert; }
protected:
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	CommodityTradeWidget *m_commodityTradeWidget;
	void OnClickBuy(int equipType);
	void OnClickSell(int equipType);
	const BBAdvert *m_advert;
	SpaceStation *m_station;
	bool m_adTaken;

	static int l_luachatform_add_goods_trader(lua_State *l);
};

#endif /* _LUACHATFORM_H */
