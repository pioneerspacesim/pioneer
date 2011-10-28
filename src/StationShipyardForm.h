#ifndef _STATIONSHIPYARDFORM_H
#define _STATIONSHIPYARDFORM_H

#include "Form.h"

class StationShipyardForm : public FaceForm {
public:
	StationShipyardForm(FormController *controller);

private:
    void EquipmentMarket();
    void Repairs();
    void ShipMarket();
};

#endif

