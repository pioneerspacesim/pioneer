#ifndef _STATIONSERVICESFORM_H
#define _STATIONSERVICESFORM_H

#include "Form.h"

class StationServicesForm : public FaceForm {
public:
	StationServicesForm(FormController *controller);

private:
	void RequestLaunch();
	void Shipyard();
	void CommodityMarket();
	void BulletinBoard();
	void Police();
};

#endif
