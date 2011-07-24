#include "StationPoliceForm.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"
#include "FaceVideoLink.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "PiLang.h"

void StationPoliceForm::OnOptionClicked(int option)
{
	switch (option) {
		case 0: {
			SpaceStation *station = Pi::player->GetDockedWith();

			SetTitle(stringf(256, PiLang::SOMEWHERE_POLICE, station->GetLabel().c_str()).c_str());

			SetFaceFlags(FaceVideoLink::ARMOUR);
			SetFaceSeed(MTRand(station->GetSBody()->seed).Int32());

			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);

			if (fine == 0) {
				SetMessage(PiLang::WE_HAVE_NO_BUSINESS_WITH_YOU);
			}
			else {
				SetMessage(stringf(256, PiLang::YOU_MUST_PAY_FINE_OF_N_CREDITS, format_money(fine).c_str()).c_str());
				AddOption(PiLang::PAY_THE_FINE_NOW, 1);
			}

			AddOption(PiLang::HANG_UP, -1);

			break;
		}

		case 1: {
			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);

			if (fine > Pi::player->GetMoney()) {
				Pi::cpan->MsgLog()->Message("", "You do not have enough money.");
				return;
			}

			Pi::player->SetMoney(Pi::player->GetMoney() - fine);
			Polit::AddCrime(0, -fine);

			Close();
			break;
		}

		default:
			Close();
			break;
	}
}
