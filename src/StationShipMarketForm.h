// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#ifndef _STATIONSHIPMARKETFORM_H
#define _STATIONSHIPMARKETFORM_H

#include "Form.h"
#include "SpaceStation.h"

class StationShipMarketForm : public FaceForm {
public:
	StationShipMarketForm(FormController *controller);
	virtual ~StationShipMarketForm();

private:
	void OnShipsForSaleChanged();
	void UpdateShipList();
	void ViewShip(int num);

	SpaceStation *m_station;
	sigc::connection m_onShipsForSaleChangedConnection;
	Gui::VBox *m_shiplistBox;
};

#endif
