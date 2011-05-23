#ifndef _SERVICESCHATFORM_H
#define _SERVICESCHATFORM_H

#include "ChatForm.h"

class ServicesChatForm : public FaceChatForm {
public:
	ServicesChatForm();

private:
	void RequestLaunch();
	void Shipyard();
	void CommodityMarket();
	void BulletinBoard();
	void Police();
};

#endif
