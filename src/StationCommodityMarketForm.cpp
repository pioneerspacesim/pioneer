#include "StationCommodityMarketForm.h"
#include "Pi.h"
#include "Player.h"
#include "ShipCpanel.h"
#include "PiLang.h"

StationCommodityMarketForm::StationCommodityMarketForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(256, PiLang::SOMEWHERE_COMMODITIES_MARKET, m_station->GetLabel().c_str()));

	m_trader = new CommodityTradeWidget(m_station);
	m_trader->onClickBuy.connect(sigc::mem_fun(this, &StationCommodityMarketForm::OnClickBuy));
	m_trader->onClickSell.connect(sigc::mem_fun(this, &StationCommodityMarketForm::OnClickSell));
	Add(m_trader, 0, 0);

	ShowAll();
}

void StationCommodityMarketForm::OnClickBuy(int commodity)
{
	if (m_station->SellTo(Pi::player, Equip::Type(commodity), true)) {
		Pi::cpan->MsgLog()->Message("", stringf(512, PiLang::BOUGHT_1T_OF, EquipType::types[commodity].name));
	}
	m_trader->UpdateStock(commodity);
}

void StationCommodityMarketForm::OnClickSell(int commodity)
{
	if (m_station->BuyFrom(Pi::player, Equip::Type(commodity), true)) {
		Pi::cpan->MsgLog()->Message("", stringf(512, PiLang::SOLD_1T_OF, EquipType::types[commodity].name));
	}
	m_trader->UpdateStock(commodity);
}
