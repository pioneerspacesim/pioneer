#include "Pi.h"
#include "Player.h"
#include "BBAdvertChatForm.h"
#include "LuaUtils.h"
#include "libs.h"
#include "Gui.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "PoliceChatForm.h"
#include "CommodityTradeWidget.h"
#include "LuaObject.h"

void BBAdvertChatForm::OnAdvertDeleted()
{
	m_advert = 0;
	Clear();
	SetMessage("Sorry, this advert has expired.");
	//AddOption("Hang up.", 0);
}

void BBAdvertChatForm::AddOption(std::string text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &BBAdvertChatForm::CallDialogHandlerTrampoline), val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}
