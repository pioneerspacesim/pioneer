// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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

