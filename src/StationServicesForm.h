#ifndef _SERVICESCHATFORM_H
#define _SERVICESCHATFORM_H

#include "Form.h"

class ServicesForm : public FaceForm {
public:
	ServicesForm();

private:
	void RequestLaunch();
	void Shipyard();
	void CommodityMarket();
	void BulletinBoard();
	void Police();
};

#endif
