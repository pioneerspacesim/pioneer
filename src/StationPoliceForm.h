#ifndef _STATIONPOLICEFORM_H
#define _STATIONPOLICEFORM_H

#include "ChatForm.h"

class StationPoliceForm : public ChatForm {
public:
	StationPoliceForm();
	virtual void OnOptionClicked(int option);
};

#endif
