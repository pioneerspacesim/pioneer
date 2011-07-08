#include "Pi.h"
#include "Sector.h"
#include "SectorView.h"
#include "SystemInfoView.h"
#include "ShipCpanel.h"
#include "Player.h"
#include "Polit.h"
#include "Space.h"
#include "SystemPath.h"

SystemInfoView::SystemInfoView()
{
	SetTransparency(true);
	m_system = 0;
	m_refresh = false;
}

void SystemInfoView::OnBodySelected(SBody *b)
{
	{
		printf("\n");
		printf("Gas, liquid, ice: %f, %f, %f\n", b->m_volatileGas.ToFloat(), b->m_volatileLiquid.ToFloat(), b->m_volatileIces.ToFloat());
	}

	SystemPath path = m_system->GetPathOf(b);
	if (Pi::currentSystem->GetPath() == m_system->GetPath()) {
		Body* body = Space::FindBodyForPath(&path);
		if(body != 0)
			Pi::player->SetNavTarget(body);
	}

	UpdateIconSelections();
}

void SystemInfoView::OnBodyViewed(SBody *b)
{
	std::string desc, data;

	m_infoBox->DeleteAllChildren();
	
	Gui::Fixed *fixed = new Gui::Fixed(600, 200);
	m_infoBox->PackStart(fixed, true);
	Gui::VBox *col1 = new Gui::VBox();
	Gui::VBox *col2 = new Gui::VBox();
	fixed->Add(col1, 0, 0);
	fixed->Add(col2, 300, 0);

#define _add_label_and_value(label, value) { \
	Gui::Label *l = (new Gui::Label(label))->Color(1.0f,1.0f,0.0f); \
	col1->PackEnd(l); \
	l = (new Gui::Label(value))->Color(1.0f,1.0f,0.0f); \
	col2->PackEnd(l); \
}

	{
		Gui::Label *l = new Gui::Label(b->name + ": " + b->GetAstroDescription());
		l->Color(1,1,0);
		m_infoBox->PackStart(l);
	}

	_add_label_and_value("Mass", stringf(64, "%.3f %s masses", b->mass.ToDouble(), 
		(b->GetSuperType() == SBody::SUPERTYPE_STAR ? "Solar" : "Earth")));

	_add_label_and_value("Surface temperature", stringf(64, "%d C", b->averageTemp-273));

	if (b->parent) {
		float days = float(b->orbit.period) / float(60*60*24);
		if (days > 1000) {
			data = stringf(64, "%.1f years", days/365);
		} else {
			data = stringf(64, "%.1f days", b->orbit.period / (60*60*24));
		}
		_add_label_and_value("Orbital period", data);
		_add_label_and_value("Periapsis distance", stringf(64, "%.3f AU", b->orbMin.ToDouble()));
		_add_label_and_value("Apoapsis distance", stringf(64, "%.3f AU", b->orbMax.ToDouble()));
		_add_label_and_value("Eccentricity", stringf(64, "%.2f", b->orbit.eccentricity));
		_add_label_and_value("Axial tilt", stringf(64, "%.1f degrees", b->axialTilt.ToDouble() * (180.0/M_PI) ));
		const float dayLen = float(b->GetRotationPeriod());
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

	m_infoBox->ShowAll();
	m_infoBox->ResizeRequest();
}

void SystemInfoView::UpdateEconomyTab()
{
	/* Economy info page */
	StarSystem *s = m_system;
	std::string data;
	
/*	if (s->m_econType) {
		data = "Economy: ";

		std::vector<std::string> v;
		if (s->m_econType & ECON_AGRICULTURE) v.push_back("Agricultural");
		if (s->m_econType & ECON_MINING) v.push_back("Mining");
		if (s->m_econType & ECON_INDUSTRY) v.push_back("Industrial");
		data += string_join(v, ", ");
		data += "\n";
	}
	m_econInfo->SetText(data);
*/
	/* imports and exports */
	std::vector<std::string> crud;
	data = "#ff0Major Imports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (s->GetCommodityBasePriceModPercent(i) > 10)
			crud.push_back(std::string("#fff")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "#777None\n";
	m_econMajImport->SetText(data);

	crud.clear();
	data = "#ff0Minor Imports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((s->GetCommodityBasePriceModPercent(i) > 2) && (s->GetCommodityBasePriceModPercent(i) <= 10))
			crud.push_back(std::string("#777")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "#777None\n";
	m_econMinImport->SetText(data);

	crud.clear();
	data = "#ff0Major Exports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (s->GetCommodityBasePriceModPercent(i) < -10)
			crud.push_back(std::string("#fff")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "#777None\n";
	m_econMajExport->SetText(data);

	crud.clear();
	data = "#ff0Minor Exports:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((s->GetCommodityBasePriceModPercent(i) < -2) && (s->GetCommodityBasePriceModPercent(i) >= -10))
			crud.push_back(std::string("#777")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "#777None\n";
	m_econMinExport->SetText(data);

	crud.clear();
	data = "#ff0Illegal Goods:\n";
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (!Polit::IsCommodityLegal(s, Equip::Type(i)))
			crud.push_back(std::string("#777")+EquipType::types[i].name);
	}
	if (crud.size()) data += string_join(crud, "\n")+"\n";
	else data += "#777None\n";
	m_econIllegal->SetText(data);

	m_econInfoTab->ResizeRequest();
}

void SystemInfoView::PutBodies(SBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, int &starports, float &prevSize)
{
	float size[2];
	float myPos[2];
	myPos[0] = pos[0];
	myPos[1] = pos[1];
	if (body->GetSuperType() == SBody::SUPERTYPE_STARPORT) starports++;
	if (body->type == SBody::TYPE_STARPORT_SURFACE) return;
	if (body->type != SBody::TYPE_GRAVPOINT) {
		BodyIcon *ib = new BodyIcon( (PIONEER_DATA_DIR "/" + std::string(body->GetIcon())).c_str() );
		m_bodyIcons.push_back(std::pair<std::string, BodyIcon*>(body->name, ib));
		ib->GetSize(size);
		if (prevSize < 0) prevSize = size[!dir];
		ib->onSelect.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodySelected), body));
		ib->onMouseEnter.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodyViewed), body));
		ib->onMouseLeave.connect(sigc::mem_fun(this, &SystemInfoView::OnSwitchTo));
		myPos[0] += (dir ? prevSize*0.5 - size[0]*0.5 : 0);
		myPos[1] += (!dir ? prevSize*0.5 - size[1]*0.5 : 0);
		container->Add(ib, myPos[0], myPos[1]);

		if (body->GetSuperType() != SBody::SUPERTYPE_STARPORT) majorBodies++;
		pos[dir] += size[dir];
		dir = !dir;
		myPos[dir] += size[dir];
	} else {
		size[0] = -1;
		size[1] = -1;
		pos[!dir] += 320;
	}

	float prevSizeForKids = size[!dir];
	for (std::vector<SBody*>::iterator i = body->children.begin();
	     i != body->children.end(); ++i) {
		PutBodies(*i, container, dir, myPos, majorBodies, starports, prevSizeForKids);
	}
}

void SystemInfoView::OnClickBackground(Gui::MouseButtonEvent *e)
{
	if (e->isdown) {
		// XXX reinit view unnecessary - we only want to show
		// the general system info text... 
		m_refresh = true;
	}
}

void SystemInfoView::SystemChanged(StarSystem *s)
{
	DeleteAllChildren();

	m_system = s;
	if (!s) return;			// Does happen

	m_sbodyInfoTab = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));

	if (s->m_unexplored) {
		Add(m_sbodyInfoTab, 0, 0);

		std::string _info =
			"Unexplored System. Star information has been gathered by remote telescope, but no planetary information is available.";

		Gui::Label *l = (new Gui::Label(_info))->Color(1.0f,1.0f,0.0f);
		m_sbodyInfoTab->Add(l, 35, 300);

		ShowAll();
		return;
	}

	m_econInfoTab = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
	Gui::Fixed *demographicsTab = new Gui::Fixed();
	
	m_tabs = new Gui::Tabbed();
	m_tabs->AddPage(new Gui::Label("Planetary info"), m_sbodyInfoTab);
	m_tabs->AddPage(new Gui::Label("Economic info"), m_econInfoTab);
	m_tabs->AddPage(new Gui::Label("Demographics"), demographicsTab);
	Add(m_tabs, 0, 0);

	m_sbodyInfoTab->onMouseButtonEvent.connect(sigc::mem_fun(this, &SystemInfoView::OnClickBackground));

	m_bodyIcons.clear();
	int majorBodies, starports;
	{
		float pos[2] = { 0, 0 };
		float psize = -1;
		majorBodies = starports = 0;
		PutBodies(s->rootBody, m_econInfoTab, 1, pos, majorBodies, starports, psize);

		majorBodies = starports = 0;
		pos[0] = pos[1] = 0;
		psize = -1;
		PutBodies(s->rootBody, m_sbodyInfoTab, 1, pos, majorBodies, starports, psize);

		majorBodies = starports = 0;
		pos[0] = pos[1] = 0;
		psize = -1;
		PutBodies(s->rootBody, demographicsTab, 1, pos, majorBodies, starports, psize);
	}
	
	std::string _info = stringf(2048, "Stable system with %d major %s and %d starport%s.\n\n%s",
		majorBodies, majorBodies == 1 ? "body" : "bodies",
		starports, starports == 1 ? "" : "s",
		std::string(s->GetLongDescription()).c_str());
	
	{
		// astronomical body info tab
		m_infoBox = new Gui::VBox();

		Gui::HBox *scrollBox = new Gui::HBox();
		scrollBox->SetSpacing(5);
		scrollBox->SetSizeRequest(730, 200);
		m_sbodyInfoTab->Add(scrollBox, 35, 300);

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(0,0);
		scroll->SetAdjustment(&portal->vscrollAdjust);
		
		Gui::Label *l = (new Gui::Label(_info))->Color(1.0f,1.0f,0.0f);
		m_infoBox->PackStart(l);
		portal->Add(m_infoBox);
		scrollBox->PackStart(scroll);
		scrollBox->PackStart(portal, true);
	}

	{
		// economy tab
		Gui::HBox *scrollBox2 = new Gui::HBox();
		scrollBox2->SetSizeRequest(730, 200);
		m_econInfoTab->Add(scrollBox2, 35, 300);
		Gui::VScrollBar *scroll2 = new Gui::VScrollBar();
		Gui::VScrollPortal *portal2 = new Gui::VScrollPortal(0,0);
		scroll2->SetAdjustment(&portal2->vscrollAdjust);
		scrollBox2->PackStart(scroll2);
		scrollBox2->PackStart(portal2, true);

		m_econInfo = new Gui::Label("");
		m_econInfoTab->Add(m_econInfo, 35, 250);

		Gui::Fixed *f = new Gui::Fixed();
		m_econMajImport = new Gui::Label("");
		m_econMinImport = new Gui::Label("");
		m_econMajExport = new Gui::Label("");
		m_econMinExport = new Gui::Label("");
		m_econIllegal = new Gui::Label("");
		m_econMajImport->Color(1,1,0);
		m_econMinImport->Color(1,1,0);
		m_econMajExport->Color(1,1,0);
		m_econMinExport->Color(1,1,0);
		m_econIllegal->Color(1,1,0);
		f->Add(m_econMajImport, 0, 0);
		f->Add(m_econMinImport, 150, 0);
		f->Add(m_econMajExport, 300, 0);
		f->Add(m_econMinExport, 450, 0);
		f->Add(m_econIllegal, 600, 0);
		portal2->Add(f);

		UpdateEconomyTab();
	}

	{
		Gui::Fixed *col1 = new Gui::Fixed();
		demographicsTab->Add(col1, 200, 350);
		Gui::Fixed *col2 = new Gui::Fixed();
		demographicsTab->Add(col2, 400, 350);
	
		const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);

		col1->Add((new Gui::Label("System type:"))->Color(1,1,0), 0, 0);
		col2->Add(new Gui::Label(m_system->GetShortDescription()), 0, 0);
		
		col1->Add((new Gui::Label("Government type:"))->Color(1,1,0), 0, YSEP);
		col2->Add(new Gui::Label(Polit::GetGovernmentDesc(m_system)), 0, YSEP);
		
		col1->Add((new Gui::Label("Economy type:"))->Color(1,1,0), 0, 2*YSEP);
		col2->Add(new Gui::Label(Polit::GetEconomicDesc(m_system)), 0, 2*YSEP);
		
		col1->Add((new Gui::Label("Allegiance:"))->Color(1,1,0), 0, 3*YSEP);
		col2->Add(new Gui::Label(Polit::GetAllegianceDesc(m_system)), 0, 3*YSEP);
		
		col1->Add((new Gui::Label("Population:"))->Color(1,1,0), 0, 4*YSEP);
		std::string popmsg;
		fixed pop = m_system->m_totalPop;
		if (pop >= fixed(1,1)) { popmsg = stringf(256, "Over %d billion", pop.ToInt32()); }
		else if (pop >= fixed(1,1000)) { popmsg = stringf(256, "Over %d million", (pop*1000).ToInt32()); }
		else if (pop != fixed(0)) { popmsg = "Only a few thousand"; }
		else { popmsg = "No registered inhabitants"; }
		col2->Add(new Gui::Label(popmsg), 0, 4*YSEP);

		SystemPath path = m_system->GetPath();
		col1->Add((new Gui::Label("Sector coordinates:"))->Color(1,1,0), 0, 5*YSEP);
		col2->Add(new Gui::Label(stringf(128, "%d, %d", path.sectorX, path.sectorY)), 0, 5*YSEP);
		col1->Add((new Gui::Label("System number:"))->Color(1,1,0), 0, 6*YSEP);
		col2->Add(new Gui::Label(stringf(128, "%d", path.systemIndex)), 0, 6*YSEP);
	}

	UpdateIconSelections();

	ShowAll();
}

void SystemInfoView::Draw3D()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SystemInfoView::Update()
{
	if (m_refresh) {
		SystemChanged(Pi::GetSelectedSystem());
		m_refresh = false;
	}
}

void SystemInfoView::OnSwitchTo()
{
	if (Pi::GetSelectedSystem() != m_system)
		m_refresh = true;
}

void SystemInfoView::NextPage()
{
	m_tabs->OnActivate();
}

void SystemInfoView::UpdateIconSelections()
{
	//navtarget can be only set in current system
	for (std::vector<std::pair<std::string, BodyIcon*> >::iterator it = m_bodyIcons.begin();
		 it != m_bodyIcons.end(); ++it) {
			 (*it).second->SetSelected(false);
		if (Pi::currentSystem->GetPath() == m_system->GetPath() &&
			Pi::player->GetNavTarget() &&
			(*it).first == Pi::player->GetNavTarget()->GetLabel())
			(*it).second->SetSelected(true);
	}
}

SystemInfoView::BodyIcon::BodyIcon(const char *img) :
	Gui::ImageRadioButton(0, img, img)
{

}

void SystemInfoView::BodyIcon::Draw()
{
	Gui::ImageRadioButton::Draw();
	if (!GetSelected()) return;
	float size[2];
	GetSize(size);
	glColor3f(0.f, 1.f, 0.f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.f, 0.f);
	glVertex2f(size[0], 0.f);
	glVertex2f(size[0], size[1]);
	glVertex2f(0.f, size[1]);
	glEnd();
}

void SystemInfoView::BodyIcon::OnActivate()
{
	//don't set pressed state here
	onSelect.emit();
}
