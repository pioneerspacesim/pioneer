#ifndef _STATIONSHIPVIEWFORM_H
#define _STATIONSHIPVIEWFORM_H

#include "Form.h"
#include "SpaceStation.h"
#include "ShipFlavour.h"
#include "LmrModel.h"

class StationShipViewForm : public BlankForm {
public:
	StationShipViewForm(FormController *controller, ShipFlavour ship);

private:
	SpaceStation *m_station;
	ShipFlavour m_flavour;
};

#endif
