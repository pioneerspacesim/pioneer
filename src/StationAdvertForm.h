#ifndef _STATIONADVERTFORM_H
#define _STATIONADVERTFORM_H

#include "ChatForm.h"
#include "SpaceStation.h"
#include "FormController.h"

class StationAdvertForm : public ChatForm {
public:
	StationAdvertForm(FormController *controller, SpaceStation *station, const BBAdvert &ad);
	~StationAdvertForm();
	
	virtual void OnOptionClicked(int option) = 0;

	bool AdTaken() { return m_adTaken; }
	void RemoveAdvertOnClose() { m_adTaken = true; }

	SpaceStation *GetStation() const { return m_station; }
	const BBAdvert *GetAdvert() const { return &m_advert; }

private:
	void OnClose(Form *form);

	bool m_adTaken;
	sigc::connection m_formClosedConnection;

	SpaceStation *m_station;
	BBAdvert m_advert;
};

#endif
