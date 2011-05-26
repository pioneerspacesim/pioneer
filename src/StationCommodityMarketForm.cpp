#include "StationCommodityMarketForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"

StationCommodityMarketForm::StationCommodityMarketForm() : FaceForm()
{
	SpaceStation *station = Pi::player->GetDockedWith();

	SetTitle(stringf(256, "%s commodity market", station->GetLabel().c_str()));

	m_trader = new CommodityTradeWidget(station);
	Add(m_trader, 0, 0);

	ShowAll();
}
