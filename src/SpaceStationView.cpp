#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "utils.h"
#include "Lang.h"
#include "ui/UIManager.h"

#include "StationServicesForm.h"

SpaceStationView::SpaceStationView(): View()
{
	SetTransparency(true);

	m_formController = new FormController();
	m_formController->onRefresh.connect(sigc::mem_fun(this, &SpaceStationView::RefreshForForm));

	// XXX hack
	Pi::uiManager->SetStashItem("player.legalStatus", Lang::CLEAN);
}

SpaceStationView::~SpaceStationView()
{
	delete m_formController;
}

void SpaceStationView::OnSwitchTo()
{
	m_formController->JumpToForm(new StationServicesForm(m_formController));
}

void SpaceStationView::RefreshForForm(Form *f)
{
	Pi::uiManager->SetStashItem("form.title", f->GetTitle());

/*
	switch (f->GetType()) {

		case Form::FACE: {
			FaceForm *form = static_cast<FaceForm*>(f);

			if (!form->GetFaceSeed())
				form->SetFaceSeed(Pi::player->GetDockedWith()->GetSBody()->seed);

			if (!m_videoLink || form->GetFaceFlags() != m_videoLink->GetFlags() ||
				form->GetFaceSeed() != m_videoLink->GetSeed()) {
				if (m_videoLink) {
					Remove(m_videoLink);
					delete m_videoLink;
				}

				m_videoLink = new FaceVideoLink(295, 285, form->GetFaceFlags(), form->GetFaceSeed(),
					form->GetCharacterName(), form->GetCharacterTitle());
				Add(m_videoLink, 10, 40);
			}
			m_videoLink->ShowAll();

			Remove(m_formStack);
			Add(m_formStack, 320, 40);

			break;
		}

		case Form::BLANK:
		default: {
			if (m_videoLink)
				m_videoLink->Hide();

			Remove(m_formStack);
			Add(m_formStack, 10, 40);

			break;
		}

	}

	m_formStack->ShowAll();
*/
}
