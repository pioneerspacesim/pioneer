#ifndef _STATIONSHIPREPAIRFORM_H
#define _STATIONSHIPREPAIRFORM_H

#include "Form.h"
#include "SpaceStation.h"

class StationShipRepairForm : public FaceForm {

public:
	StationShipRepairForm(FormController *controller);

	virtual void ShowAll();

private:
	int GetRepairCost(float percent);
	void RepairHull(float percent);
	void UpdateLabels();

	SpaceStation *m_station;

	Gui::Label *m_working;

	Gui::HBox *m_tableBox;
	Gui::Label *m_repairOneCost;
	Gui::Label *m_repairAllCost;
	Gui::Label *m_repairAllDesc;
};

#endif

