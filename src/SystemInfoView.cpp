#include "Pi.h"
#include "Sector.h"
#include "SectorView.h"
#include "SystemInfoView.h"
#include "ShipCpanel.h"

SystemInfoView::SystemInfoView(): GenericSystemView()
{
	SetTransparency(true);
	m_bodySelected = 0;
	onSelectedSystemChanged.connect(sigc::mem_fun(this, &SystemInfoView::SystemChanged));
}

void SystemInfoView::OnBodySelected(StarSystem::SBody *b)
{
	m_bodySelected = b;

	std::string desc, data;

	char buf[1024];

	desc += stringf(256, "%s: %s\n", b->name.c_str(), b->GetAstroDescription());
	data += "\n";

	desc += "Mass\n";
	data += stringf(64, "%.2f %s masses\n", b->mass.ToDouble(), 
		(b->GetSuperType() == StarSystem::SUPERTYPE_STAR ? "Solar" : "Earth"));

	desc += "Surface temperature\n";
	data += stringf(64, "%d C\n", b->averageTemp-273);

	// surface temperature
	// major starports
	// orbital period
	// orbital radius
	// ecc, incl

	if (b->parent) {
		float days = b->orbit.period / (60*60*24);
		desc += "Orbital period\n";
		if (days > 1000) {
			data += stringf(64, "%.1f years\n", days/365);
		} else {
			data += stringf(64, "%.1f days\n", b->orbit.period / (60*60*24));
		}
		desc += "Perihelion distance\n";
		data += stringf(64, "%.2f AU\n", b->orbMin.ToDouble());
		desc += "Aphelion distance\n";
		data += stringf(64, "%.2f AU\n", b->orbMax.ToDouble());
		desc += "Eccentricity\n";
		data += stringf(64, "%.2f\n", b->orbit.eccentricity);
		const float dayLen = b->GetRotationPeriod();
		if (dayLen) {
			desc += "Day length\n";
			data += stringf(64, "%.1f earth days\n", dayLen/(60*60*24));
		}
		int numSurfaceStarports = 0;
		std::string nameList;
		for (std::vector<StarSystem::SBody*>::iterator i = b->children.begin(); i != b->children.end(); ++i) {
			if ((*i)->type == StarSystem::TYPE_STARPORT_SURFACE) {
				nameList += (numSurfaceStarports ? ", " : "") + (*i)->name;
				numSurfaceStarports++;
			}
		}
		if (numSurfaceStarports) {
			desc += "Starports\n";
			data += nameList+"\n";
		}
	}

	m_infoLabel->SetText(desc);
	m_infoData->SetText(data);
	
	/* Economy info page */
	desc = stringf(256, "%s: %s\n", b->name.c_str(), b->GetAstroDescription());
	data = "\n";
	
	if (b->econType) {
		desc += "Economy\n";

		std::vector<std::string> v;
		if (b->econType & ECON_AGRICULTURE) v.push_back("Agricultural");
		if (b->econType & ECON_MINING) v.push_back("Mining");
		if (b->econType & ECON_INDUSTRY) v.push_back("Industrial");
		data += string_join(v, ", ");
		data += "\n";
	}

	/* imports and exports */
	std::vector<std::string> crud;
	desc += "\n";
	data += "\n";
	desc += "#ff0Imports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (b->tradeLevel[i]) printf("Trade %s at %d%%\n", EquipType::types[i].name, b->tradeLevel[i]);
	}
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (b->tradeLevel[i] > 5)
			crud.push_back(std::string("#fff")+EquipType::types[i].name);
	}
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((b->tradeLevel[i] > 0) && (b->tradeLevel[i] <= 5))
			crud.push_back(std::string("#777")+EquipType::types[i].name);
	}
	if (crud.size()) desc += string_join(crud, "\n")+"\n";
	else desc += "None\n";
	crud.clear();

	data += "#ff0Exports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (b->tradeLevel[i] < -5)
			crud.push_back(std::string("#fff")+EquipType::types[i].name);
	}
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((b->tradeLevel[i] < 0) && (b->tradeLevel[i] >= -5))
			crud.push_back(std::string("#777")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "None\n";
	crud.clear();
	data += " #ff0";

	m_econLabel->SetText(desc);
	m_econData->SetText(data);
}

void SystemInfoView::PutBodies(StarSystem::SBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, float prevSize)
{
	float size[2];
	float myPos[2];
	myPos[0] = pos[0];
	myPos[1] = pos[1];
	if (body->type == StarSystem::TYPE_STARPORT_SURFACE) return;
	if (body->type != StarSystem::TYPE_GRAVPOINT) {
		Gui::ImageButton *ib = new Gui::ImageButton(body->GetIcon());
		ib->GetSize(size);
		if (prevSize == -1) prevSize = size[!dir];
		ib->onClick.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodySelected), body));
		myPos[0] += (dir ? prevSize*0.5 - size[0]*0.5 : 0);
		myPos[1] += (!dir ? prevSize*0.5 - size[1]*0.5 : 0);
		container->Add(ib, myPos[0],
			myPos[1]);
		if (container == m_econInfoTab) {
			/* Grey out planets with no human habitation */
			if (body->econType == 0) ib->SetEnabled(false);
		}
		majorBodies++;
		pos[dir] += size[dir];
		dir = !dir;
		myPos[dir] += size[dir];
	} else {
		size[0] = -1;
		size[1] = -1;
		pos[!dir] += 320;
	}

	for (std::vector<StarSystem::SBody*>::iterator i = body->children.begin();
	     i != body->children.end(); ++i) {
		PutBodies(*i, container, dir, myPos, majorBodies, size[!dir]);
	}
}

void SystemInfoView::SystemChanged(StarSystem *s)
{
	DeleteAllChildren();
	
	m_sbodyInfoTab = new Gui::Fixed(Gui::Screen::GetWidth(),Gui::Screen::GetHeight());
	m_econInfoTab = new Gui::Fixed(Gui::Screen::GetWidth(), Gui::Screen::GetHeight());
	
	Gui::Tabbed *tabbed = new Gui::Tabbed();
	tabbed->AddPage(new Gui::Label("Planetary info"), m_sbodyInfoTab);
	tabbed->AddPage(new Gui::Label("Economic info"), m_econInfoTab);
	Add(tabbed, 0, 0);
	
	{
		int majorBodies = 0;
		float pos[2] = { 0, 0 };
		PutBodies(s->rootBody, m_econInfoTab, 1, pos, majorBodies, -1);
	}

	int majorBodies = 0;
	float pos[2] = { 0, 0 };
	PutBodies(s->rootBody, m_sbodyInfoTab, 1, pos, majorBodies, -1);
	
	float size[2];
	GetSize(size);
	printf("size %f,%f\n", size[0], size[1]);
	
	char buf[512];
	snprintf(buf, sizeof(buf), "Stable system with %d major bodies.", majorBodies);
	m_infoLabel = new Gui::Label(buf);
	m_infoLabel->SetColor(1,1,0);
	m_sbodyInfoTab->Add(m_infoLabel, 50, 350);

	m_infoData = new Gui::Label("");
	m_infoData->SetColor(1,1,0);
	m_sbodyInfoTab->Add(m_infoData, 300, 350);
	m_sbodyInfoTab->ShowAll();
	
	m_econLabel = new Gui::Label("");
	m_econLabel->SetColor(1,1,0);
	m_econInfoTab->Add(m_econLabel, 50, 300);
	m_econData = new Gui::Label("");
	m_econData->SetColor(1,1,0);
	m_econInfoTab->Add(m_econData, 300, 300);

	m_econInfoTab->ShowAll();
	
	ShowAll();
}

void SystemInfoView::Draw3D()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GenericSystemView::Draw3D();
}

void SystemInfoView::Update()
{

}

