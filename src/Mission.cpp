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
	wr_int((int)m_status);
	_Save();
}

Mission *Mission::Load()
{
	using namespace Serializer::Read;
	int type = rd_int();

	Mission *m = missionFactoryFn[type].Create(type);
	m->m_agreedPayoff = rd_int();
	m->m_status = (MissionState)rd_int();
	m->_Load();
	return m;
}

MissionChatForm::MissionChatForm()
{
	m_msgregion = new Gui::VBox();
	m_optregion = new Gui::VBox();
       	m_msgregion->SetSpacing(5.0f);
       	m_optregion->SetSpacing(5.0f);
	Add(m_msgregion, 0, 0);
	Add(m_optregion, 0, 150);
	m_msgregion->Show();
	m_optregion->Show();
	hasOpts = false;
	Clear();
}

void MissionChatForm::Clear()
{
	m_msgregion->DeleteAllChildren();
	m_optregion->DeleteAllChildren();
}

void MissionChatForm::Message(const char *msg)
{
	m_msgregion->PackEnd(new Gui::Label(msg));
	ShowAll();
}

void MissionChatForm::AddOption(Mission *m, const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(m, &Mission::FormResponse), this, val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
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

