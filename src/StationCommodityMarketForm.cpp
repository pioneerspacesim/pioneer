#include "StationCommodityMarketForm.h"
#include "Pi.h"
#include "Player.h"
#include "ShipCpanel.h"
#include "Lang.h"
#include "StringF.h"

StationCommodityMarketForm::StationCommodityMarketForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(Lang::SOMEWHERE_COMMODITIES_MARKET, formatarg("station", m_station->GetLabel())));

	m_trader = new CommodityTradeWidget(m_station);
	m_trader->onClickBuy.connect(sigc::mem_fun(this, &StationCommodityMarketForm::OnClickBuy));
	m_trader->onClickSell.connect(sigc::mem_fun(this, &StationCommodityMarketForm::OnClickSell));
	Add(m_trader, 0, 0);

	ShowAll();
}

void StationCommodityMarketForm::OnClickBuy(int commodity)
{
	if (m_station->SellTo(Pi::player, Equip::Type(commodity), true)) {
		Pi::cpan->MsgLog()->Message("", stringf(Lang::BOUGHT_1T_OF, formatarg("commodity", Equip::types[commodity].name)));
	}
	m_trader->UpdateStock(commodity);
}

void StationCommodityMarketForm::OnClickSell(int commodity)
{
	if (m_station->BuyFrom(Pi::player, Equip::Type(commodity), true)) {
		Pi::cpan->MsgLog()->Message("", stringf(Lang::SOLD_1T_OF, formatarg("commodity", Equip::types[commodity].name)));
	}
	m_trader->UpdateStock(commodity);
}
