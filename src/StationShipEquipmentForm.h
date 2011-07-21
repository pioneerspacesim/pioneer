#ifndef _STATIONSHIPEQUIPMENTFORM_H
#define _STATIONSHIPEQUIPMENTFORM_H

#include "Form.h"
#include "SpaceStation.h"

class PickLaserMountForm;

class StationShipEquipmentForm : public FaceForm {
	friend class PickLaserMountForm;

public:
	StationShipEquipmentForm(FormController *controller);

	virtual void ShowAll();

private:
	void FitItem(Equip::Type t);
	void RemoveItem(Equip::Type t);

	void FitItemForce(Equip::Type t, int pos = -1);
	void RemoveItemForce(Equip::Type t, int pos = -1);

	void RecalcButtonVisibility();

	struct ButtonPair {
		Equip::Type  type;
		Gui::Button *add;
		Gui::Button *remove;
	};

	std::list<ButtonPair> m_buttons;

	SpaceStation *m_station;
};

#endif
