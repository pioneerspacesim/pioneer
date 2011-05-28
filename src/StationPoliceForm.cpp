#include "StationPoliceForm.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"
#include "FaceVideoLink.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"

void StationPoliceForm::OnOptionClicked(int option)
{
	switch (option) {
		case 0: {
			SetTitle(stringf(256, "%s Police", Pi::player->GetDockedWith()->GetLabel().c_str()).c_str());

			SetFaceFlags(FaceVideoLink::ARMOUR);

			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);

			if (fine == 0) {
				SetMessage("We have no business with you at the moment.");
			}
			else {
				SetMessage(stringf(256, "We do not tolerate crime. You must pay a fine of %s.", format_money(fine).c_str()).c_str());
				AddOption("Pay the fine now.", 1);
			}

			AddOption("Hang up.", -1);

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

            m_formController->CloseForm();
			break;
		}

		default:
            m_formController->CloseForm();
			break;
	}
}
