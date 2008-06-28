#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"

InfoView::InfoView(): View()
{
	SetTransparency(false);
	SetBgColor(.0,.2,.4);

	float size[2];
	GetSize(size);
	
	info1 = new Gui::Label("some crap starshit");
	Add(info1, 40, size[1]-40);
}

void InfoView::UpdateInfo()
{
	char buf[512];
	std::string nfo;
	const ShipType &stype = Pi::player->GetShipType();
	nfo = "SHIP INFORMATION:  "+std::string(stype.name);
	
	Equip::Type e = Pi::player->m_equipment.Get(Equip::SLOT_ENGINE);
	nfo += std::string("\n\nDrive system:      ")+EquipType::types[e].name;

	shipstats_t stats;
	Pi::player->CalcStats(&stats);
	snprintf(buf, sizeof(buf), "\n\nCapacity:          %dt\n"
				       "Free:              %dt\n"
			               "Used:              %dt\n"
				       "All-up weight:     %dt", stats.max_capacity,
			stats.free_capacity, stats.used_capacity, stats.total_mass);
	nfo += std::string(buf);

	snprintf(buf, sizeof(buf), "\n\nHyperspace range:  %.2f light years", stats.hyperspace_range);
	nfo += std::string(buf);

	info1->SetText(nfo);
}

void InfoView::Draw3D()
{
}

void InfoView::Update()
{
}
