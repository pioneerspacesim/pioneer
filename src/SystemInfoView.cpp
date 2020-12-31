// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemInfoView.h"

#include "Game.h"
#include "Lang.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Space.h"
#include "StringF.h"
#include "galaxy/Economy.h"
#include "galaxy/Factions.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Polit.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "graphics/Renderer.h"
#include <functional>

SystemInfoView::SystemInfoView(Game *game) :
	PiGuiView("system-info"),
	m_game(game)
{
	SetTransparency(true);
	m_refresh = REFRESH_NONE;
	m_unexplored = true;
}

void SystemInfoView::OnBodySelected(SystemBody *b)
{
	{
		Output("\n");
		Output("Gas, liquid, ice: %f, %f, %f\n", b->GetVolatileGas(), b->GetVolatileLiquid(), b->GetVolatileIces());
	}

	SystemPath path = m_system->GetPathOf(b);
	RefCountedPtr<StarSystem> currentSys = m_game->GetSpace()->GetStarSystem();
	bool isCurrentSystem = (currentSys && currentSys->GetPath() == m_system->GetPath());

	if (path == m_selectedBodyPath) {
		if (isCurrentSystem) {
			Pi::player->SetNavTarget(0);
		}
	} else {
		if (isCurrentSystem) {
			Body *body = m_game->GetSpace()->FindBodyForPath(&path);
			if (body != 0)
				Pi::player->SetNavTarget(body);
		} else if (b->IsJumpable()) {
			m_game->GetSectorView()->SwitchToPath(path);
		}
	}

	UpdateIconSelections();
}

void SystemInfoView::OnBodyViewed(SystemBody *b)
{
	std::string data;

	m_infoBox->DeleteAllChildren();

	Gui::Fixed *outer = new Gui::Fixed(600, 200);
	m_infoBox->PackStart(outer);
	Gui::VBox *col1 = new Gui::VBox();
	Gui::VBox *col2 = new Gui::VBox();
	outer->Add(col1, 0, 0);
	outer->Add(col2, 300, 0);

#define _add_label_and_value(label, value)                           \
	{                                                                \
		Gui::Label *l = (new Gui::Label(label))->Color(255, 255, 0); \
		col1->PackEnd(l);                                            \
		l = (new Gui::Label(value))->Color(255, 255, 0);             \
		col2->PackEnd(l);                                            \
	}

	bool multiple = (b->GetSuperType() == SystemBody::SUPERTYPE_STAR &&
		b->GetParent() && b->GetParent()->GetType() == SystemBody::TYPE_GRAVPOINT && b->GetParent()->GetParent());
	{
		Gui::Label *l = new Gui::Label(b->GetName() + ": " + b->GetAstroDescription() +
			(multiple ? (std::string(" (") + b->GetParent()->GetName() + ")") : ""));
		l->Color(255, 255, 0);
		m_infoBox->PackStart(l);
	}

	if (b->GetType() != SystemBody::TYPE_STARPORT_ORBITAL) {
		_add_label_and_value(Lang::MASS, stringf(b->GetSuperType() == SystemBody::SUPERTYPE_STAR ? Lang::N_SOLAR_MASSES : Lang::N_EARTH_MASSES, formatarg("mass", b->GetMassAsFixed().ToDouble())));

		_add_label_and_value(Lang::RADIUS, stringf(b->GetSuperType() == SystemBody::SUPERTYPE_STAR ? Lang::N_SOLAR_RADII : Lang::N_EARTH_RADII, formatarg("radius", b->GetRadiusAsFixed().ToDouble()), formatarg("radkm", b->GetRadius() / 1000.0)));
	}

	if (b->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		_add_label_and_value(Lang::EQUATORIAL_RADIUS_TO_POLAR_RADIUS_RATIO, stringf("%0{f.3}", b->GetAspectRatio()));
	}

	if (b->GetType() != SystemBody::TYPE_STARPORT_ORBITAL) {
		_add_label_and_value(Lang::SURFACE_TEMPERATURE, stringf(Lang::N_CELSIUS, formatarg("temperature", b->GetAverageTemp() - 273)));
		static const auto earthG = G * EARTH_MASS / (EARTH_RADIUS * EARTH_RADIUS);
		const auto surfAccel = b->CalcSurfaceGravity();
		_add_label_and_value(Lang::SURFACE_GRAVITY, stringf("%0{f.3} m/s^2 (%1{f.3} g)", surfAccel, surfAccel / earthG));
	}

	if (b->GetParent()) {
		float days = float(b->GetOrbit().Period()) / float(60 * 60 * 24);
		if (days > 1000) {
			data = stringf(Lang::N_YEARS, formatarg("years", days / 365));
		} else {
			data = stringf(Lang::N_DAYS, formatarg("days", b->GetOrbit().Period() / (60 * 60 * 24)));
		}
		if (multiple) {
			float pdays = float(b->GetParent()->GetOrbit().Period()) / float(60 * 60 * 24);
			data += " (" + (pdays > 1000 ? stringf(Lang::N_YEARS, formatarg("years", pdays / 365)) : stringf(Lang::N_DAYS, formatarg("days", b->GetParent()->GetOrbit().Period() / (60 * 60 * 24)))) + ")";
		}
		_add_label_and_value(Lang::ORBITAL_PERIOD, data);
		_add_label_and_value(Lang::PERIAPSIS_DISTANCE, format_distance(b->GetOrbMin() * AU, 3) + (multiple ? (std::string(" (") + format_distance(b->GetParent()->GetOrbMin() * AU, 3) + ")") : ""));
		_add_label_and_value(Lang::APOAPSIS_DISTANCE, format_distance(b->GetOrbMax() * AU, 3) + (multiple ? (std::string(" (") + format_distance(b->GetParent()->GetOrbMax() * AU, 3) + ")") : ""));
		_add_label_and_value(Lang::ECCENTRICITY, stringf("%0{f.2}", b->GetOrbit().GetEccentricity()) + (multiple ? (std::string(" (") + stringf("%0{f.2}", b->GetParent()->GetOrbit().GetEccentricity()) + ")") : ""));
		if (b->GetType() != SystemBody::TYPE_STARPORT_ORBITAL) {
			_add_label_and_value(Lang::AXIAL_TILT, stringf(Lang::N_DEGREES, formatarg("angle", b->GetAxialTilt() * (180.0 / M_PI))));
			if (b->IsRotating()) {
				_add_label_and_value(
					std::string(Lang::DAY_LENGTH) + std::string(Lang::ROTATIONAL_PERIOD),
					stringf(Lang::N_EARTH_DAYS, formatarg("days", b->GetRotationPeriodInDays())));
			}
		}
		int numSurfaceStarports = 0;
		std::string nameList;
		for (const SystemBody *kid : b->GetChildren()) {
			if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
				nameList += (numSurfaceStarports ? ", " : "") + kid->GetName();
				numSurfaceStarports++;
			}
		}
		if (numSurfaceStarports) {
			_add_label_and_value(Lang::STARPORTS, nameList);
		}
	}

	m_infoBox->ShowAll();
	m_infoBox->ResizeRequest();
}

void SystemInfoView::PutBodies(SystemBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, int &starports, int &onSurface, float &prevSize)
{
	float size[2];
	float myPos[2];
	myPos[0] = pos[0];
	myPos[1] = pos[1];
	if (body->GetSuperType() == SystemBody::SUPERTYPE_STARPORT) starports++;
	if (body->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
		onSurface++;
		return;
	}
	if (body->GetType() != SystemBody::TYPE_GRAVPOINT) {
		BodyIcon *ib = new BodyIcon(body->GetIcon(), m_renderer);
		if (body->GetSuperType() == SystemBody::SUPERTYPE_ROCKY_PLANET) {
			for (const SystemBody *kid : body->GetChildren()) {
				if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
					ib->SetHasStarport();
					break;
				}
			}
		}
		m_bodyIcons.push_back(std::pair<Uint32, BodyIcon *>(body->GetPath().bodyIndex, ib));
		ib->GetSize(size);
		if (prevSize < 0) prevSize = size[!dir];
		ib->onSelect.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodySelected), body));
		ib->onMouseEnter.connect(sigc::bind(sigc::mem_fun(this, &SystemInfoView::OnBodyViewed), body));
		ib->onMouseLeave.connect(sigc::mem_fun(this, &SystemInfoView::OnSwitchTo));
		myPos[0] += (dir ? prevSize * 0.5 - size[0] * 0.5 : 0);
		myPos[1] += (!dir ? prevSize * 0.5 - size[1] * 0.5 : 0);
		container->Add(ib, myPos[0], myPos[1]);

		if (body->GetSuperType() != SystemBody::SUPERTYPE_STARPORT) majorBodies++;
		pos[dir] += size[dir];
		dir = !dir;
		myPos[dir] += size[dir];
	} else {
		size[0] = -1;
		size[1] = -1;
		pos[!dir] += 400;
	}

	float prevSizeForKids = size[!dir];
	for (SystemBody *kid : body->GetChildren()) {
		PutBodies(kid, container, dir, myPos, majorBodies, starports, onSurface, prevSizeForKids);
	}
}

void SystemInfoView::OnClickBackground(Gui::MouseButtonEvent *e)
{
	if (e->isdown && e->button == SDL_BUTTON_LEFT) {
		// XXX reinit view unnecessary - we only want to show
		// the general system info text...
		m_refresh = REFRESH_ALL;
	}
}

void SystemInfoView::SystemChanged(const SystemPath &path)
{
	DeleteAllChildren();
	m_tabs = 0;
	m_bodyIcons.clear();

	if (!path.HasValidSystem())
		return;

	m_system = m_game->GetGalaxy()->GetStarSystem(path);
	m_unexplored = m_system->GetUnexplored();
	m_sbodyInfoTab = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight() - 100));

	if (m_system->GetUnexplored()) {
		Add(m_sbodyInfoTab, 0, 0);

		std::string _info =
			Lang::UNEXPLORED_SYSTEM_STAR_INFO_ONLY;

		Gui::Label *l = (new Gui::Label(_info))->Color(255, 255, 0);
		m_sbodyInfoTab->Add(l, 35, 300);
		m_selectedBodyPath = SystemPath();

		ShowAll();
		return;
	}

	m_tabs = new Gui::Tabbed();
	m_tabs->AddPage(new Gui::Label(Lang::PLANETARY_INFO), m_sbodyInfoTab);
	Add(m_tabs, 0, 0);

	m_sbodyInfoTab->onMouseButtonEvent.connect(sigc::mem_fun(this, &SystemInfoView::OnClickBackground));

	int majorBodies, starports, onSurface;
	{
		float pos[2] = { 0, 0 };
		float psize = -1;
		majorBodies = starports = onSurface = 0;
		PutBodies(m_system->GetRootBody().Get(), m_sbodyInfoTab, 1, pos, majorBodies, starports, onSurface, psize);
	}

	std::string _info = stringf(
		Lang::STABLE_SYSTEM_WITH_N_MAJOR_BODIES_STARPORTS,
		formatarg("bodycount", majorBodies),
		formatarg("body(s)", std::string(majorBodies == 1 ? Lang::BODY : Lang::BODIES)),
		formatarg("portcount", starports),
		formatarg("starport(s)", std::string(starports == 1 ? Lang::STARPORT : Lang::COUNT_STARPORTS)));
	if (starports > 0)
		_info += stringf(Lang::COUNT_ON_SURFACE, formatarg("surfacecount", onSurface));
	_info += ".\n\n";
	_info += m_system->GetLongDescription();

	{
		// astronomical body info tab
		m_infoBox = new Gui::VBox();

		Gui::HBox *scrollBox = new Gui::HBox();
		scrollBox->SetSpacing(5);
		m_sbodyInfoTab->Add(scrollBox, 35, 250);

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(730);
		scroll->SetAdjustment(&portal->vscrollAdjust);

		Gui::Label *l = (new Gui::Label(_info))->Color(255, 255, 0);
		m_infoBox->PackStart(l);
		portal->Add(m_infoBox);
		scrollBox->PackStart(scroll);
		scrollBox->PackStart(portal);
	}

	UpdateIconSelections();

	ShowAll();
}

void SystemInfoView::Draw3D()
{
	PROFILE_SCOPED()
	m_renderer->SetTransform(matrix4x4f::Identity());
	m_renderer->ClearScreen();
}

static bool IsShownInInfoView(const SystemBody *sb)
{
	if (!sb) return false; // sanity check
	SystemBody::BodySuperType superType = sb->GetSuperType();
	return superType == SystemBody::SUPERTYPE_STAR || superType == SystemBody::SUPERTYPE_GAS_GIANT ||
		superType == SystemBody::SUPERTYPE_ROCKY_PLANET ||
		sb->GetType() == SystemBody::TYPE_STARPORT_ORBITAL;
}

SystemInfoView::RefreshType SystemInfoView::NeedsRefresh()
{
	if (!m_system || !m_game->GetSectorView()->GetSelected().IsSameSystem(m_system->GetPath()))
		return REFRESH_ALL;

	if (m_system->GetUnexplored() != m_unexplored)
		return REFRESH_ALL;

	if (m_system->GetUnexplored())
		return REFRESH_NONE; // Nothing can be selected and we reset in SystemChanged

	RefCountedPtr<StarSystem> currentSys = m_game->GetSpace()->GetStarSystem();
	if (!currentSys || currentSys->GetPath() != m_system->GetPath()) {
		// We are not currently in the selected system
		if (m_selectedBodyPath.IsBodyPath()) {
			// Some body was selected
			if (m_game->GetSectorView()->GetSelected() != m_selectedBodyPath)
				return REFRESH_SELECTED_BODY; // but now we want a different body (or none at all)
		} else {
			// No body was selected
			if (m_game->GetSectorView()->GetSelected().IsBodyPath())
				// but now we want one, this can only be a star,
				// so no check for IsShownInInfoView() needed
				return REFRESH_SELECTED_BODY;
		}
	} else {
		Body *navTarget = Pi::player->GetNavTarget();
		if (navTarget && (navTarget->GetSystemBody() != nullptr) && IsShownInInfoView(navTarget->GetSystemBody())) {
			// Navigation target is something we show in the info view
			if (navTarget->GetSystemBody()->GetPath() != m_selectedBodyPath)
				return REFRESH_SELECTED_BODY; // and wasn't selected, yet
		} else {
			// nothing to be selected
			if (m_selectedBodyPath.IsBodyPath())
				return REFRESH_SELECTED_BODY; // but there was something selected
		}
	}

	return REFRESH_NONE;
}

void SystemInfoView::Update()
{
	switch (m_refresh) {
	case REFRESH_ALL:
		SystemChanged(m_game->GetSectorView()->GetSelected());
		m_refresh = REFRESH_NONE;
		assert(NeedsRefresh() == REFRESH_NONE);
		break;
	case REFRESH_SELECTED_BODY:
		UpdateIconSelections();
		m_refresh = REFRESH_NONE;
		assert(NeedsRefresh() == REFRESH_NONE);
		break;
	case REFRESH_NONE:
		break;
	}
}

void SystemInfoView::OnSwitchTo()
{
	if (m_refresh != REFRESH_ALL) {
		RefreshType needsRefresh = NeedsRefresh();
		if (needsRefresh != REFRESH_NONE)
			m_refresh = needsRefresh;
	}
}

void SystemInfoView::UpdateIconSelections()
{
	m_selectedBodyPath = SystemPath();

	for (auto &bodyIcon : m_bodyIcons) {

		bodyIcon.second->SetSelected(false);

		RefCountedPtr<StarSystem> currentSys = m_game->GetSpace()->GetStarSystem();
		if (currentSys && currentSys->GetPath() == m_system->GetPath()) {
			//navtarget can be only set in current system
			Body *navtarget = Pi::player->GetNavTarget();
			if (navtarget &&
				(navtarget->IsType(ObjectType::STAR) ||
					navtarget->IsType(ObjectType::PLANET) ||
					navtarget->IsType(ObjectType::SPACESTATION))) {
				const SystemPath &navpath = navtarget->GetSystemBody()->GetPath();
				if (bodyIcon.first == navpath.bodyIndex) {
					bodyIcon.second->SetSelectColor(Color(0, 255, 0, 255));
					bodyIcon.second->SetSelected(true);
					m_selectedBodyPath = navpath;
				}
			}
		} else {
			SystemPath selected = m_game->GetSectorView()->GetSelected();
			if (selected.IsSameSystem(m_system->GetPath()) && !selected.IsSystemPath()) {
				if (bodyIcon.first == selected.bodyIndex) {
					bodyIcon.second->SetSelectColor(Color(64, 96, 255, 255));
					bodyIcon.second->SetSelected(true);
					m_selectedBodyPath = selected;
				}
			}
		}
	}
}

SystemInfoView::BodyIcon::BodyIcon(const char *img, Graphics::Renderer *r) :
	Gui::ImageRadioButton(0, img, img),
	m_renderer(r),
	m_hasStarport(false),
	m_selectColor(0, 255, 0, 255)
{
	//no blending
	Graphics::RenderStateDesc rsd;
	m_renderState = r->CreateRenderState(rsd);

	float size[2];
	GetSize(size);
	const vector3f vts[] = {
		vector3f(0.f, 0.f, 0.f),
		vector3f(size[0], 0.f, 0.f),
		vector3f(size[0], size[1], 0.f),
		vector3f(0.f, size[1], 0.f),
	};
	m_selectBox.SetData(COUNTOF(vts), vts, m_selectColor);

	static const Color portColor = Color(64, 128, 128, 255);
	// The -0.1f offset seems to be the best compromise to make the circles closed (e.g. around Mars), symmetric, fitting with selection
	// and not overlapping to much with asteroids
	m_circle.reset(new Graphics::Drawables::Circle(m_renderer, size[0] * 0.5f, size[0] * 0.5f - 0.1f, size[1] * 0.5f, 0.f, portColor, m_renderState));
}

void SystemInfoView::BodyIcon::Draw()
{
	Gui::ImageRadioButton::Draw();
	if (!GetSelected() && !HasStarport()) return;
	float size[2];
	GetSize(size);
	if (HasStarport()) {
		m_circle->Draw(m_renderer);
	}
	if (GetSelected()) {
		m_selectBox.Draw(m_renderer, m_renderState, Graphics::LINE_LOOP);
	}
}

void SystemInfoView::BodyIcon::OnActivate()
{
	//don't set pressed state here
	onSelect.emit();
}
