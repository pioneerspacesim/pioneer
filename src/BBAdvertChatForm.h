#ifndef _BBADVERTCHATFORM_H
#define _BBADVERTCHATFORM_H

#include "GenericChatForm.h"

class BBAdvert;
class SpaceStation;
class CommodityTradeWidget;

class BBAdvertChatForm: public GenericChatForm {
public:
	BBAdvertChatForm(SpaceStation *station, const BBAdvert *ad) : m_station(station), m_advert(ad), m_adTaken(false) {}
	virtual ~BBAdvertChatForm();

	void AddOption(std::string text, int val);
	virtual void CallDialogHandler(int optionClicked) = 0;
	virtual void RemoveAdvert() {}
	void RemoveAdvertOnClose() { m_adTaken = true; }
	void OnAdvertDeleted();
	void GotoPolice();

	const BBAdvert *GetAdvert() const { return m_advert; }
	SpaceStation *GetStation() const { return m_station; }

private:
	void CallDialogHandlerTrampoline(int optionClicked) {
		this->CallDialogHandler(optionClicked);
	}

	SpaceStation *m_station;
	const BBAdvert *m_advert;

	bool m_adTaken;
};

#endif /* _BBADVERTCHATFORM_H */
