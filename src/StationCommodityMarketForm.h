#ifndef _STATIONCOMMODITYMARKETFORM_H
#define _STATIONCOMMODITYMARKETFORM_H

#include "Form.h"
#include "CommodityTradeWidget.h"
#include "SpaceStation.h"

class StationCommodityMarketForm : public FaceForm {
public:
	StationCommodityMarketForm(FormController *controller);

private:
	void OnClickBuy(int commodity);
	void OnClickSell(int commodity);

	SpaceStation *m_station;
	CommodityTradeWidget *m_trader;
};

#endif
