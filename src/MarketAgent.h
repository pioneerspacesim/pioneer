#ifndef _MARKETAGENT_H
#define _MARKETAGENT_H

#include "libs.h"
#include "ShipType.h"

class MarketAgent {
public:
	MarketAgent(): m_money(0) {}
	void SetMoney(Sint64 m) { m_money = m; }
	Sint64 GetMoney() { return m_money; }
	bool Pay(MarketAgent *b, Sint64 amount);
	/**
	 * SellTo() and BuyFrom() use the commodity price of this MarketAgent.
	 */
	bool SellTo(MarketAgent *other, Equip::Type t);
	bool BuyFrom(MarketAgent *other, Equip::Type t);
	virtual Sint64 GetPrice(Equip::Type t) const = 0;
	virtual bool CanBuy(Equip::Type t) const = 0;
	// can sell means do we have enough stock
	virtual bool CanSell(Equip::Type t) const = 0;
	// does sell means do we bother with this commodity?
	virtual bool DoesSell(Equip::Type t) const = 0;
	virtual int GetStock(Equip::Type t) const = 0;
protected:
	virtual void Bought(Equip::Type t) = 0;
	virtual void Sold(Equip::Type t) = 0;
	void Load();
	void Save() const;
private:
	Sint64 m_money;
};

#endif /* _MARKETAGENT_H */
