#ifndef _STATIONCOMMODITYMARKETFORM_H
#define _STATIONCOMMODITYMARKETFORM_H

#include "Form.h"
#include "CommodityTradeWidget.h"

class StationCommodityMarketForm : public FaceForm {
public:
	StationCommodityMarketForm();

private:
	CommodityTradeWidget *m_trader;
};

#endif
