#ifndef _STATIONPOLICEFORM_H
#define _STATIONPOLICEFORM_H

#include "ChatForm.h"

class StationPoliceForm : public ChatForm {
public:
	StationPoliceForm(FormController *controller);
	virtual void OnOptionClicked(int option);
};

#endif
