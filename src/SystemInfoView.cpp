#include "Pi.h"
#include "Sector.h"
#include "SectorView.h"
#include "SystemInfoView.h"
#include "ShipCpanel.h"
#include "Player.h"

SystemInfoView::SystemInfoView(): GenericSystemView()
{
	SetTransparency(true);
	m_bodySelected = 0;
	onSelectedSystemChanged.connect(sigc::mem_fun(this, &SystemInfoView::SystemChanged));
}

void SystemInfoView::OnBodySelected(SBody *b)
{
	m_bodySelected = b;

	SBodyPath path;
	m_system->GetPathOf(b, &path);
	Pi::player->SetHyperspaceTarget(&path);

	std::string desc, data;

	char buf[1024];
	m_infoBox->DeleteAllChildren();
	
	Gui::Fixed *fixed = new Gui::Fixed(600, 10);
	m_infoBox->PackStart(fixed, true);
	Gui::VBox *col1 = new Gui::VBox();
	Gui::VBox *col2 = new Gui::VBox();
	fixed->Add(col1, 0, 0);
	fixed->Add(col2, 300, 0);

#define _add_label_and_value(label, value) { \
	Gui::Label *l = new Gui::Label(label); \
	l->SetColor(1,1,0); \
	col1->PackEnd(l); \
	l = new Gui::Label(value); \
	l->SetColor(1,1,0); \
	col2->PackEnd(l); \
}

	{
		Gui::Label *l = new Gui::Label(stringf(256, "%s: %s", b->name.c_str(), b->GetAstroDescription()));
		l->SetColor(1,1,0);
		m_infoBox->PackStart(l);
	}

	_add_label_and_value("Mass", stringf(64, "%.2f %s masses", b->mass.ToDouble(), 
		(b->GetSuperType() == SBody::SUPERTYPE_STAR ? "Solar" : "Earth")));

	_add_label_and_value("Surface temperature", stringf(64, "%d C", b->averageTemp-273));

	if (b->parent) {
		float days = b->orbit.period / (60*60*24);
		if (days > 1000) {
			data = stringf(64, "%.1f years", days/365);
		} else {
			data = stringf(64, "%.1f days", b->orbit.period / (60*60*24));
		}
		_add_label_and_value("Orbital period", data);
		_add_label_and_value("Perihelion distance", stringf(64, "%.2f AU", b->orbMin.ToDouble()));
		_add_label_and_value("Aphelion distance", stringf(64, "%.2f AU", b->orbMax.ToDouble()));
		_add_label_and_value("Eccentricity", stringf(64, "%.2f", b->orbit.eccentricity));
		const float dayLen = b->GetRotationPeriod();
		if (dayLen) {
			_add_label_and_value("Day length", stringf(64, "%.1f earth days", dayLen/(60*60*24)));
		}
		int numSurfaceStarports = 0;
		std::string nameList;
		for (std::vector<SBody*>::iterator i = b->children.begin(); i != b->children.end(); ++i) {
			if ((*i)->type == SBody::TYPE_STARPORT_SURFACE) {
				nameList += (numSurfaceStarports ? ", " : "") + (*i)->name;
				numSurfaceStarports++;
			}
		}
		if (numSurfaceStarports) {
			_add_label_and_value("Starports", nameList);
		}
	}

	fixed->ShowAll();
	col1->ShowAll();
	col2->ShowAll();
	m_infoBox->ShowAll();
	m_infoBox->ResizeRequest();
	
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

void SystemInfoView::PutBodies(SBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, float prevSize)
{
	float size[2];
	float myPos[2];
	myPos[0] = pos[0];
	myPos[1] = pos[1];
	if (body->type == SBody::TYPE_STARPORT_SURFACE) return;
	if (body->type != SBody::TYPE_GRAVPOINT) {
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

	for (std::vector<SBody*>::iterator i = body->children.begin();
	     i != body->children.end(); ++i) {
		PutBodies(*i, container, dir, myPos, majorBodies, size[!dir]);
	}
}

void SystemInfoView::SystemChanged(StarSystem *s)
{
	DeleteAllChildren();
	
	m_system = s;
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
	
	char buf[512];
	snprintf(buf, sizeof(buf), "Stable system with %d major bodies.", majorBodies);
	std::string _info = buf + std::string("\n\n") + std::string(s->GetLongDescription());
	
	m_infoBox = new Gui::VBox();

	Gui::HBox *scrollBox = new Gui::HBox();
	scrollBox->SetSizeRequest(730, 200);
	m_sbodyInfoTab->Add(scrollBox, 35, 300);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(0,0);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	
	Gui::Label *l = new Gui::Label(_info);
	l->SetColor(1,1,0);
	m_infoBox->PackStart(l);
	m_infoBox->ShowAll();
	portal->Add(m_infoBox);
	portal->ShowAll();
	scrollBox->PackStart(scroll);
	scrollBox->PackStart(portal, true);
	scrollBox->ShowAll();

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

