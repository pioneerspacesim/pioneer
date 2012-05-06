#include "StationPoliceForm.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"
#include "FaceVideoLink.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "Lang.h"
#include "StringF.h"

void StationPoliceForm::OnOptionClicked(int option)
{
	switch (option) {
		case 0: {
			SpaceStation *station = Pi::player->GetDockedWith();

			SetTitle(stringf(Lang::SOMEWHERE_POLICE, formatarg("station", station->GetLabel())));

			SetFaceFlags(FaceVideoLink::ARMOUR);
			SetFaceSeed(MTRand(station->GetSystemBody()->seed).Int32());

			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);

			if (fine == 0) {
				SetMessage(Lang::WE_HAVE_NO_BUSINESS_WITH_YOU);
			}
			else {
				SetMessage(stringf(Lang::YOU_MUST_PAY_FINE_OF_N_CREDITS, formatarg("fine", format_money(fine))));
				AddOption(Lang::PAY_THE_FINE_NOW, 1);
			}

			AddOption(Lang::HANG_UP, -1);

			break;
		}

		case 1: {
			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);

			if (fine > Pi::player->GetMoney()) {
				Pi::cpan->MsgLog()->Message("", Lang::YOU_NOT_ENOUGH_MONEY);
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
