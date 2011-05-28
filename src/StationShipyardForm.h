#ifndef _STATIONSHIPYARDFORM_H
#define _STATIONSHIPYARDFORM_H

#include "Form.h"

class StationShipyardForm : public FaceForm {
public:
	StationShipyardForm();

private:
    void EquipmentMarket();
    void Servicing();
    void ShipMarket();
};

#endif

