#include "MarketAgent.h"
#include "Serializer.h"

void MarketAgent::Load(Serializer::Reader &rd)
{
	m_money = rd.Int64();
}

void MarketAgent::Save(Serializer::Writer &wr) const
{
	wr.Int64(m_money);
}

bool MarketAgent::SellTo(MarketAgent *other, Equip::Type t)
{
	if (other->CanBuy(t) && CanSell(t) && other->Pay(this, GetPrice(t))) {
		Sold(t);
		other->Bought(t);
		return true;
	} else return false;
}
	
bool MarketAgent::BuyFrom(MarketAgent *other, Equip::Type t)
{
	if (other->CanSell(t) && CanBuy(t) && Pay(other, GetPrice(t))) {
		other->Sold(t);
		Bought(t);
		return true;
	} else return false;
}
	
bool MarketAgent::Pay(MarketAgent *b, Sint64 amount) {
	if (m_money < amount) return false;
	b->m_money += amount;
	m_money -= amount;
	return true;
}
