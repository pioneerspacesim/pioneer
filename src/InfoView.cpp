#include "InfoView.h"
#include "Pi.h"
#include "ui/UIManager.h"

InfoView::InfoView(): View()
{
	SetTransparency(true);
}

void InfoView::NextPage()
{
	// change
}

void InfoView::OnSwitchTo()
{
	Pi::uiManager->OpenScreen("ship_info");
}
