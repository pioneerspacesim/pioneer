#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "Mission.h"
#include "missions/register.h"
#include "Serializer.h"
#include "Sector.h"

Mission *Mission::GenerateRandom()
{
	for (;;) {
		int type = Pi::rng.Int32(MISSION_MAX);
		const MissionFactoryDef *def = &missionFactoryFn[type];
		if (def->Create == 0) continue;
		if (Pi::rng.Int32(def->rarity) == 0) {
			Mission *m = def->Create(type);
			try {
				m->Randomize();
			} catch (CouldNotMakeMissionException) {
				delete m;
				throw;
			}
			return m;
		}
	}
}

void Mission::GiveToPlayer()
{
	Pi::player->TakeMission(this);
	Pi::player->GetDockedWith()->BBRemoveMission(this);
	AttachToPlayer();
}

void Mission::Save()
{
	using namespace Serializer::Write;
	wr_int(type);
	wr_int(m_agreedPayoff);
	_Save();
}

Mission *Mission::Load()
{
	using namespace Serializer::Read;
	int type = rd_int();

	Mission *m = missionFactoryFn[type].Create(type);
	m->m_agreedPayoff = rd_int();
	m->_Load();
	return m;
}

void MissionChatForm::Message(const char *msg)
{
	PackEnd(new Gui::Label(msg));
	ShowAll();
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
	ShowAll();
}

std::string Mission::NaturalSystemName(const SBodyPath &p)
{
	Sector s(p.sectorX, p.sectorY);
	return stringf(512, "the %s system", s.m_systems[p.systemIdx].name.c_str());
}

std::string Mission::NaturalSpaceStationName(const SBodyPath &p)
{
	Sector s(p.sectorX, p.sectorY);
	StarSystem sys(p.sectorX, p.sectorY, p.systemIdx);
	SBody *starport = sys.GetBodyByPath(&p);
	return stringf(512, "%s in the %s system [%d,%d]",
			starport->name.c_str(),
			s.m_systems[p.systemIdx].name.c_str(),
			p.sectorX, p.sectorY);
}

