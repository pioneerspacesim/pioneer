#include "StationAdvertForm.h"

StationAdvertForm::StationAdvertForm(FormController *controller, SpaceStation *station, const BBAdvert &ad) :
	ChatForm(controller), m_adTaken(false), m_station(station), m_advert(ad)
{
	m_formClosedConnection = m_formController->onClose.connect(sigc::mem_fun(this, &StationAdvertForm::OnClose));
}

StationAdvertForm::~StationAdvertForm()
{
	m_formClosedConnection.disconnect();
}

void StationAdvertForm::OnClose(Form *form)
{
	if (m_adTaken)
		m_station->RemoveBBAdvert(m_advert.ref);
}
