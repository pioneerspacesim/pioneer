#include "StationAdvertForm.h"

void StationAdvertForm::OnClose()
{
	ChatForm::OnClose();

	if (m_adTaken)
		m_station->RemoveBBAdvert(m_advert.ref);
}
