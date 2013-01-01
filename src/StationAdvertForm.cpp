// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StationAdvertForm.h"

void StationAdvertForm::OnClose()
{
	ChatForm::OnClose();

	if (m_adTaken)
		m_station->RemoveBBAdvert(m_advert.ref);
}
