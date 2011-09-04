#ifndef _STATIONSHIPVIEWFORM_H
#define _STATIONSHIPVIEWFORM_H

#include "Form.h"
#include "SpaceStation.h"
#include "ShipFlavour.h"
#include "LmrModel.h"

class StationShipViewForm : public BlankForm {
public:
	StationShipViewForm(FormController *controller, int marketIndex);

private:
	void BuyShip();

	SpaceStation *m_station;

	int m_marketIndex;
	ShipFlavour m_flavour;
};

#endif
