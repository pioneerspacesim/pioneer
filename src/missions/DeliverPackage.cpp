#include "../libs.h"
#include "DeliverPackage.h"
#include "../Serializer.h"
#include "../Pi.h"
#include "../Player.h"
#include "../ShipCpanel.h"
#include "../NameGenerator.h"

#define MAX_FLAVOUR 3

static struct dtype_t {
	const char *bbtext;
	const char *introtext;
	const char *whysomuchdoshtext;
	int danger;
} deliveryType[MAX_FLAVOUR] = {
	{
		"WANTED. Delivery of a package to %s.",
		"Hello. I'm %s. I'm willing to pay $%d for a ship to carry a package to %s.",
		"It is nothing special.",
		0
	}, {
		"URGENT. Fast ship needed to deliver a package to %s.",
		"Hello. I'm %s. I'm willing to pay $%d for a ship to carry a package to %s.",
		"It is nothing special.",
		0
	}, {
		"DELIVERY. Documents to the %s system. $%d to an experienced pilot.",
		"Hello. I'm %s. I'm willing to pay $%d for a ship to carry a package to %s.",
		"Some extremely sensitive documents have fallen into my hands, and I have "
		"reason to believe that the leak has been traced to me.",
		150
	}
};

double GetGoodDate()
{
	return floor(Pi::GetGameTime()/(60.0*60.0*24.0))*(60.0*60.0*24.0)
		+ 60.0*60.0*24.0*(double)Pi::rng.Int32(4,31) - 1;
}

void DeliverPackage::Randomize()
{
	m_flavour = Pi::rng.Int32(MAX_FLAVOUR);
	m_personGender = (bool)Pi::rng.Int32(2);
	m_personName = NameGenerator::FullName(Pi::rng, m_personGender);
	m_basePay = (20000 + Pi::rng.Int32(80000)) * (100+deliveryType[m_flavour].danger) / 100;
	m_deadline = GetGoodDate();
	if (!Pi::currentSystem->GetRandomStarportNearButNotIn(Pi::rng, &m_dest)) {
		throw CouldNotMakeMissionException();
	}
}

std::string DeliverPackage::GetMissionText()
{
	return stringf(1024, "Deliver a package to %s by %s.",
			NaturalSpaceStationName(m_dest).c_str(), format_date_only(m_deadline).c_str());
}

std::string DeliverPackage::GetBulletinBoardText()
{
	return stringf(1024, deliveryType[m_flavour].bbtext, NaturalSystemName(m_dest).c_str(),
			m_basePay);
}

void DeliverPackage::StartChat(MissionChatForm *form)
{
	form->Message(stringf(2048, deliveryType[m_flavour].introtext,
				m_personName.c_str(),
				m_basePay,
				NaturalSpaceStationName(m_dest).c_str()
				).c_str());
	PutOptions(form);
}

void DeliverPackage::PutOptions(MissionChatForm *form)
{
	form->AddOption(this, "Why so much money?", 1);
	form->AddOption(this, "Could you repeat the original request?", 2);
	form->AddOption(this, "How soon must it be delivered?", 4);
	form->AddOption(this, "Ok, agreed. (Hang up)", 3);
	form->AddOption(this, "Hang up.", 0);
}

void DeliverPackage::FormResponse(MissionChatForm *form, int resp)
{
	if (resp==0) {
		form->Close();
		return;
	}
	form->Clear();
	if (resp == 1) {
		form->Message(deliveryType[m_flavour].whysomuchdoshtext);
		PutOptions(form);
	}
	else if (resp == 2) {
		StartChat(form);
	}
	else if (resp == 3) {
		m_agreedPayoff = m_basePay;
		GiveToPlayer();
		return;
	}
	else if (resp == 4) {
		form->Message(stringf(1024, "It must be delivered by %s.", format_date_only(m_deadline).c_str()).c_str());
		PutOptions(form);
	}
}

void DeliverPackage::AttachToPlayer()
{
	Pi::player->onDock.connect(sigc::mem_fun(this, &DeliverPackage::TestCompleted));
}

void DeliverPackage::TestCompleted()
{
	SpaceStation *station = Pi::player->GetDockedWith();
// need to compare with desired sbodypath m_dest, but no means to do so yet...
	printf("DID YOU DOCK WITH WHAT I WANTED DAMN YOU?? %d,%d,%d,\n", m_dest.sectorX, m_dest.sectorY, m_dest.systemIdx);

}

void DeliverPackage::_Load()
{
	using namespace Serializer::Read;
	m_flavour = rd_int();
	m_personGender = rd_bool();
	m_personName = rd_string();
	m_basePay = rd_int();
	m_deadline = rd_double();
	SBodyPath::Unserialize(&m_dest);
}

void DeliverPackage::_Save()
{
	using namespace Serializer::Write;
	wr_int(m_flavour);
	wr_bool(m_personGender);
	wr_string(m_personName);
	wr_int(m_basePay);
	wr_double(m_deadline);
	m_dest.Serialize();
}

