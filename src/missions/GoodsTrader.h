#ifndef _MISSION_GOODSTRADER_H
#define _MISSION_GOODSTRADER_H

#include "../Mission.h"
#include "../Gui.h"
#include "../EquipType.h"
#include "../MarketAgent.h"

class CommodityTradeWidget;

class GoodsTrader: public Mission, public MarketAgent {
public:
	GoodsTrader(int type);
	virtual void Randomize();
	virtual std::string GetBulletinBoardText();
	virtual void StartChat(GenericChatForm *);
	virtual void FormResponse(GenericChatForm*, int);
	/* MarketAgent stuff */
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t) const;
	bool CanSell(Equip::Type t) const;
	bool DoesSell(Equip::Type t) const;
	int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }

	static Mission* Create(int type) {
		return new GoodsTrader(type);
	}
protected:
	virtual void _Load();
	virtual void _Save();
	std::string m_name;
	int m_bbtextidx;
	int m_priceIncPercent;
	int m_equipmentStock[Equip::TYPE_MAX];
	
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	void OnClickBuy(int equip_type, GenericChatForm *form);
	void OnClickSell(int equip_type, GenericChatForm *form);
	CommodityTradeWidget *m_commodityTradeWidget;
};


#endif /* _MISSION_GOODSTRADER_H */
