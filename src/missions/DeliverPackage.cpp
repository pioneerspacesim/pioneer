#include "../libs.h"
#include "DeliverPackage.h"
#include "../Serializer.h"
#include "../Pi.h"
#include "../Player.h"
#include "../ShipCpanel.h"
#include "../NameGenerator.h"
#include "../SpaceStation.h"

#define MAX_FLAVOUR 4

/*
 * TODO:
 * Consequences (besides not being paid) for failure.
 * Reputation.
 * Make 'danger' do something.
 * Add options like begging for half dosh in advance.
 * Idea: sneaky mission purporting to be stolen secret documents from
 * military. On delivery you get a big fine or get kicked out of military if
 * you are in it. Some means of telling which are fake (destination?)
 */

static struct dtype_t {
	const char *bbtext;
	const char *introtext;
	const char *whysomuchdoshtext;
	const char *successmsg;
	const char *failuremsg;
	/* percent */
	int danger;
	int time;
	int money;
} deliveryType[MAX_FLAVOUR] = {
	{
		"GOING TO %0? Money paid for delivery of a small package.",
		"Hi, I'm %0. I'll pay you %1 if you will deliver a small package to %2.",
		"When a friend visited me she left behind some clothes and antique paper books. I'd like to "
		"have them returned to her.",
		"Thank you for the delivery. You have been paid in full.",
		"Jesus wept, you took forever over that delivery. I'm not willing to pay you.",
		0,
		300,
		50,
	}, {
		"WANTED. Delivery of a package to %0.",
		"Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		"It is nothing special.",
		"The package has been received and you have been paid in full.",
		"I'm frustrated by the late delivery of my package, and I refuse to pay you.",
		0,
		100,
		100,
	}, {
		"URGENT. Fast ship needed to deliver a package to %0.",
		"Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		"It is a research proposal and must be delivered by the deadline or we may not get funding.",
		"You have been paid in full for the delivery. Thank you.",
		"I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
		0,
		75,
		110,
	}, {
		"DELIVERY. Documents to the %0 system. %1 to an experienced pilot.",
		"Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		"Some extremely sensitive documents have fallen into my hands, and I have "
		"reason to believe that the leak has been traced to me.",
		"Your timely and discrete service is much appreciated. You have been paid in full.",
		"Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		150,
		75,
		250,
	}
};



double GetGoodDate(int maxDays)
{
	return floor(Pi::GetGameTime()/(60.0*60.0*24.0))*(60.0*60.0*24.0)
		+ 60.0*60.0*24.0*(double)Pi::rng.Int32(4,maxDays) - 1;
}

void DeliverPackage::Randomize()
{
	m_flavour = Pi::rng.Int32(MAX_FLAVOUR);
	m_personGender = (bool)Pi::rng.Int32(2);
	m_personName = NameGenerator::FullName(Pi::rng, m_personGender);
	m_basePay = (20000 + Pi::rng.Int32(80000)) * (100+deliveryType[m_flavour].money) / 100;
	m_deadline = GetGoodDate(deliveryType[m_flavour].time * 31 / 100);
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
	std::string args[2] = { NaturalSystemName(m_dest), format_money(m_basePay) };
	return string_subst(deliveryType[m_flavour].bbtext, 2, args);
}

void DeliverPackage::StartChat(GenericChatForm *form)
{
	std::string args[3] = { m_personName, format_money(m_basePay), NaturalSpaceStationName(m_dest) };
	form->Message(string_subst(deliveryType[m_flavour].introtext, 3, args).c_str());
	PutOptions(form);
}

void DeliverPackage::PutOptions(GenericChatForm *form)
{
	sigc::slot<void,GenericChatForm*,int> slot = sigc::mem_fun(this, &DeliverPackage::FormResponse);
	form->AddOption(slot, "Why so much money?", 1);
	form->AddOption(slot, "Could you repeat the original request?", 2);
	form->AddOption(slot, "How soon must it be delivered?", 4);
	form->AddOption(slot, "Ok, agreed. (Hang up)", 3);
	form->AddOption(slot, "Hang up.", 0);
}

void DeliverPackage::FormResponse(GenericChatForm *form, int resp)
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
	if (m_status == ACTIVE) {
		m_onDockConn = Pi::player->onDock.connect(sigc::mem_fun(this, &DeliverPackage::TestCompleted));
	}
}

void DeliverPackage::TestCompleted()
{
	SpaceStation *station = Pi::player->GetDockedWith();
	SBodyPath path;
	Pi::currentSystem->GetPathOf(station->GetSBody(), &path);
	if (path == m_dest) {
		m_onDockConn.disconnect();
		// got to destination
		if (Pi::GetGameTime() > m_deadline) {
			Pi::cpan->MsgLog()->ImportantMessage(m_personName, deliveryType[m_flavour].failuremsg);
			m_status = FAILED;
		} else {
			Pi::cpan->MsgLog()->ImportantMessage(m_personName, deliveryType[m_flavour].successmsg);
			Pi::player->SetMoney(Pi::player->GetMoney() + m_agreedPayoff);
			m_status = COMPLETED;
		}
		Pi::onPlayerMissionListChanged.emit();
	}
	else if (Pi::GetGameTime() > m_deadline) {
		m_status = FAILED;
		Pi::onPlayerMissionListChanged.emit();
	}
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

