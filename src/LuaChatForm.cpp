#include "LuaChatForm.h"
#include "libs.h"
#include "Gui.h"
#include "SpaceStation.h"
#include "PiLuaModules.h"

EXPORT_OOLUA_FUNCTIONS_6_NON_CONST(LuaChatForm, Close, Clear, SetTitle, SetMessage, AddOption, SetStage)
EXPORT_OOLUA_FUNCTIONS_1_CONST(LuaChatForm, GetStage)

void LuaChatForm::StartChat(const BBAdvert *a)
{
	m_modName = a->GetModule();
	m_modRef = a->GetLuaRef();
	m_stage = 0;
	CallDialogHandler(0);
}

void LuaChatForm::AddOption(const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &LuaChatForm::CallDialogHandler), val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}

void LuaChatForm::CallDialogHandler(int optionClicked)
{
	printf("CallDialogHandler()\n");
	lua_State *l = PiLuaModules::GetLuaState();

	lua_getglobal(l, m_modName.c_str());
	lua_getfield(l, -1, "DialogHandler");
	lua_pushvalue(l, -2); // push self
	OOLUA::push2lua(l, this);
	OOLUA::push2lua(l, m_modRef);
	OOLUA::push2lua(l, optionClicked);
	lua_call(l, 4, 0);
	lua_pop(l, 1);
}
