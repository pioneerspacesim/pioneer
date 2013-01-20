// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATIONPOLICEFORM_H
#define _STATIONPOLICEFORM_H

#include "ChatForm.h"

class StationPoliceForm : public ChatForm {
public:
	StationPoliceForm(FormController *controller) : ChatForm(controller) {}
	virtual void OnOptionClicked(int option);
};

#endif
