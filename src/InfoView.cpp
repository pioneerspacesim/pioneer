#include "InfoView.h"
#include "Pi.h"
#include "rocket/RocketManager.h"

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
	Pi::rocketManager->OpenScreen("ship_info");
}
