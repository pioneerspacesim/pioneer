#include "libs.h"
#include "Pi.h"
#include "Mission.h"
#include "missions/register.h"
#include "Serializer.h"

Mission *Mission::GenerateRandom()
{
	for (;;) {
		int type = Pi::rng.Int32(MISSION_MAX);
		const MissionFactoryDef *def = &missionFactoryFn[type];
		if (def->Create == 0) continue;
		if (Pi::rng.Int32(def->rarity) == 0) {
			Mission *m = def->Create(type);
			m->Randomize();
			return m;
		}
	}
}

void Mission::Save()
{
	using namespace Serializer::Write;
	wr_int(type);
	_Save();
}

Mission *Mission::Load()
{
	using namespace Serializer::Read;
	int type = rd_int();

	Mission *m = missionFactoryFn[type].Create(type);
	m->_Load();
	return m;
}

void MissionChatForm::Message(const char *msg)
{
	PackEnd(new Gui::Label(msg));
}

void MissionChatForm::AddOption(Mission *m, const char *text, int val)
{
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(m, &Mission::FormResponse), this, val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	PackEnd(box);
}
