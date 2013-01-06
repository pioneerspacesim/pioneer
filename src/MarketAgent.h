// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MARKETAGENT_H
#define _MARKETAGENT_H

#include "libs.h"
#include "ShipType.h"
#include "Serializer.h"

class MarketAgent {
public:
	MarketAgent(): m_money(0) {}
	void SetMoney(Sint64 m) { m_money = m; }
	Sint64 GetMoney() const { return m_money; }
	bool Pay(MarketAgent *b, Sint64 amount, bool verbose = false);
	/**
	 * SellTo() and BuyFrom() use the commodity price of this MarketAgent.
	 * When verbose=true then functions should make cpanel messages about
	 * why they failed ("You have no space", etc)
	 */
	bool SellTo(MarketAgent *other, Equip::Type t, bool verbose = false);
	bool BuyFrom(MarketAgent *other, Equip::Type t, bool verbose = false);
	virtual Sint64 GetPrice(Equip::Type t) const = 0;
	virtual bool CanBuy(Equip::Type t, bool verbose = false) const = 0;
	// can sell means do we have enough stock
	virtual bool CanSell(Equip::Type t, bool verbose = false) const = 0;
	// does sell means do we bother with this commodity?
	virtual bool DoesSell(Equip::Type t) const = 0;
	virtual int GetStock(Equip::Type t) const = 0;
protected:
	virtual void Bought(Equip::Type t) = 0;
	virtual void Sold(Equip::Type t) = 0;
	void Load(Serializer::Reader &rd);
	void Save(Serializer::Writer &wr) const;
private:
	Sint64 m_money;
};

#endif /* _MARKETAGENT_H */
