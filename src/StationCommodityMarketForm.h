// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

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
