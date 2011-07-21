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
