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

	std::string desc;

	char buf[1024];

	snprintf(buf, sizeof(buf), "%s: %s\n"
		"Mass                       %.2f %s masses\n",
		b->name.c_str(), b->GetAstroDescription(), b->mass.ToDouble(),
		(b->GetSuperType() == StarSystem::SUPERTYPE_STAR ? "Solar" : "Earth"));
	desc += buf;

	snprintf(buf, sizeof(buf), "Surface temperature        %d C\n", b->averageTemp-273);
	desc += buf;

	// surface temperature
	// major starports
	// orbital period
	// orbital radius
	// ecc, incl

	if (b->parent) {
		float days = b->orbit.period / (60*60*24);
		if (days > 1000) {
			snprintf(buf, sizeof(buf), "Orbital period             %.1f years\n", days / 365);
		} else {
			snprintf(buf, sizeof(buf), "Orbital period             %.1f days\n", b->orbit.period / (60*60*24));
		}
		desc += buf;
		snprintf(buf, sizeof(buf), "Perihelion distance        %.2f AU\n", b->orbMin.ToDouble());
		desc += buf;
		snprintf(buf, sizeof(buf), "Aphelion distance          %.2f AU\n", b->orbMax.ToDouble());
		desc += buf;
		snprintf(buf, sizeof(buf), "Eccentricity               %.2f\n", b->orbit.eccentricity);
		desc += buf;
		const float dayLen = b->GetRotationPeriod();
		if (dayLen) {
			snprintf(buf, sizeof(buf), "Day length                 %.1f earth days\n", dayLen/(60*60*24));
			desc += buf;
		}
	}

	m_infoText->SetText(desc);
}

void SystemInfoView::PutBodies(StarSystem::SBody *body, int dir, float pos[2], int &majorBodies, float prevSize)
{
	float size[2];
	float myPos[2];
	myPos[0] = pos[0];
	myPos[1] = pos[1];
	if (body->type != StarSystem::TYPE_GRAVPOINT) {
		Gui::ImageButton *ib = new Gui::ImageButton(body->GetIcon());
		ib->GetSize(size);
		size[1] = -size[1];
		if (prevSize == -1) prevSize = size[!dir];
		ib->onClick.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodySelected), body));
		myPos[0] += (dir ? prevSize*0.5 - size[0]*0.5 : 0);
		myPos[1] += (!dir ? prevSize*0.5 - size[1]*0.5 : 0);
		Add(ib, myPos[0],
			myPos[1]+size[1]);
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
		PutBodies(*i, dir, myPos, majorBodies, size[!dir]);
	}
}

void SystemInfoView::SystemChanged(StarSystem *s)
{
	DeleteAllChildren();
	float csize[2];
	int majorBodies = 0;
	GetSize(csize);

	float pos[2];
	pos[0] = 0;
	pos[1] = csize[1];

	PutBodies(s->rootBody, 1, pos, majorBodies, -1);
	
	char buf[512];
	snprintf(buf, sizeof(buf), "Stable system with %d major bodies.", majorBodies);
	m_infoText = new Gui::Label(buf);
	m_infoText->SetColor(1,1,0);
	Add(m_infoText, 50, 200);
	
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

