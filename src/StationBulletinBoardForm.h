#ifndef _STATIONBULLETINBOARDFORM_H
#define _STATIONBULLETINBOARDFORM_H

#include "Form.h"
#include "SpaceStation.h"

class StationBulletinBoardForm : public FaceForm {
public:
	StationBulletinBoardForm(FormController *controller);
	~StationBulletinBoardForm();

private:
	void OnBulletinBoardChanged();
	void OnBulletinBoardAdvertDeleted(const BBAdvert &ad);
	void UpdateAdverts();
	void ActivateAdvertForm(const BBAdvert &ad);

	SpaceStation *m_station;
	Gui::Fixed *m_advertbox;

	sigc::connection m_bbChangedConnection;
	sigc::connection m_bbAdvertDeletedConnection;
};

#endif
