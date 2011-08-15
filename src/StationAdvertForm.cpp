#include "StationAdvertForm.h"

void StationAdvertForm::OnClose()
{
	if (m_adTaken)
		m_station->RemoveBBAdvert(m_advert.ref);
}
