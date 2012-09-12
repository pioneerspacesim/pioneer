// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

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
