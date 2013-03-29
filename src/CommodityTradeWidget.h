// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COMMODITYTRADEWIDGET_H
#define _COMMODITYTRADEWIDGET_H

#include "gui/Gui.h"
#include "MarketAgent.h"
#include <map>

class CommodityTradeWidget : public Gui::VBox {
public:
	CommodityTradeWidget(MarketAgent *seller);
	void ShowAll();
	void UpdateStock(int commodity_type);
	sigc::signal<void,int> onClickSell;
	sigc::signal<void,int> onClickBuy;

private:
	void OnClickBuy(int commodity_type) {
		onClickBuy.emit(commodity_type);
	}
	void OnClickSell(int commodity_type) {
		onClickSell.emit(commodity_type);
	}

	std::map<int, Gui::Label*> m_stockLabels;
	std::map<int, Gui::Label*> m_cargoLabels;
	MarketAgent *m_seller;
};

#endif /* _COMMODITYTRADEWIDGET_H */
