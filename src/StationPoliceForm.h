#ifndef _STATIONPOLICEFORM_H
#define _STATIONPOLICEFORM_H

#include "ChatForm.h"

class StationPoliceForm : public ChatForm {
public:
	StationPoliceForm(FormController *controller) : ChatForm(controller) {}
	virtual void OnOptionClicked(int option);
};

#endif
