#include "StationPoliceForm.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"
#include "FaceVideoLink.h"

StationPoliceForm::StationPoliceForm() : FaceForm()
{
	SetTitle(stringf(256, "%s Police", Pi::player->GetDockedWith()->GetLabel().c_str()).c_str());

	SetFaceFlags(FaceVideoLink::ARMOUR);
}
