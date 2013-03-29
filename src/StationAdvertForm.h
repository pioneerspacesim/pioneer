// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATIONADVERTFORM_H
#define _STATIONADVERTFORM_H

#include "ChatForm.h"
#include "SpaceStation.h"
#include "FormController.h"

class StationAdvertForm : public ChatForm {
public:
	StationAdvertForm(FormController *controller, SpaceStation *station, const BBAdvert &ad) :
		ChatForm(controller), m_adTaken(false), m_station(station), m_advert(ad) { }

	virtual void OnOptionClicked(int option) = 0;

	bool AdTaken() { return m_adTaken; }
	void RemoveAdvertOnClose() { m_adTaken = true; }

	SpaceStation *GetStation() const { return m_station; }
	const BBAdvert *GetAdvert() const { return &m_advert; }

	virtual void OnClose();

private:
	bool m_adTaken;
	sigc::connection m_formClosedConnection;

	SpaceStation *m_station;
	BBAdvert m_advert;
};

#endif
