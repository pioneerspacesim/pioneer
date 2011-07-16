#ifndef _STATIONSHIPEQUIPMENTFORM_H
#define _STATIONSHIPEQUIPMENTFORM_H

#include "Form.h"
#include "SpaceStation.h"

class StationShipEquipmentForm : public FaceForm {
public:
	StationShipEquipmentForm(FormController *controller);

private:
	void FitItem(Equip::Type t);
	void RemoveItem(Equip::Type t);

	SpaceStation *m_station;
};

#endif
