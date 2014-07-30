// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemView.h"
#include "Pi.h"
#include "SectorView.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "Lang.h"
#include "StringF.h"
#include "Space.h"
#include "Player.h"
#include "FloatComparison.h"
#include "Game.h"
#include "AnimationCurves.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"

using namespace Graphics;

const double SystemView::PICK_OBJECT_RECT_SIZE = 12.0;
static const float MIN_ZOOM = 1e-30f;		// Just to avoid having 0
static const float MAX_ZOOM = 1e30f;
static const float ZOOM_IN_SPEED = 2;
static const float ZOOM_OUT_SPEED = 1.f/ZOOM_IN_SPEED;
static const float WHEEL_SENSITIVITY = .1f;		// Should be a variable in user settings.
// i don't know how to name it
static const double ROUGH_SIZE_OF_TURD = 10.0;


TransferPlanner::TransferPlanner() {
	m_dvPrograde = 0.0;
	m_dvNormal = 0.0;
	m_dvRadial = 0.0;
	m_factor = 1;
}

vector3d TransferPlanner::GetVel() { return Pi::player->GetVelocity() + GetOffsetVel(); }

vector3d TransferPlanner::GetOffsetVel() {
	const vector3d pPos    = Pi::player->GetPosition();
	const vector3d pVel    = Pi::player->GetVelocity();
	const vector3d pNormal = pPos.Cross(pVel);

	return m_dvPrograde * pVel.Normalized()    +
	       m_dvNormal   * pNormal.Normalized() +
	       m_dvRadial   * pPos.Normalized();
}

void TransferPlanner::AddDv(BurnDirection d, double dv) {
	switch (d) {
	case PROGRADE: m_dvPrograde += m_factor * dv; break;
	case NORMAL:   m_dvNormal   += m_factor * dv; break;
	case RADIAL:   m_dvRadial   += m_factor * dv; break;
	}
}

void TransferPlanner::ResetDv(BurnDirection d) {
	switch (d) {
	case PROGRADE: m_dvPrograde = 0; break;
	case NORMAL:   m_dvNormal   = 0; break;
	case RADIAL:   m_dvRadial   = 0; break;
	}
}

std::string TransferPlanner::printDv(BurnDirection d) {
	double dv = 0;
	char buf[10];

	switch(d) {
	case PROGRADE: dv = m_dvPrograde; break;
	case NORMAL:   dv = m_dvNormal;   break;
	case RADIAL:   dv = m_dvRadial;   break;
	}

	snprintf(buf, sizeof(buf), "%6.0fm/s", dv);
	return std::string(buf);
}

void TransferPlanner::IncreaseFactor(void) {
	if(m_factor > 1000) return;
	m_factor *= m_factorFactor;
}
void TransferPlanner::ResetFactor(void) { m_factor = 1; }

void TransferPlanner::DecreaseFactor(void) {
	if(m_factor < 0.0002) return;
	m_factor /= m_factorFactor;
}

std::string TransferPlanner::printFactor(void) {
	char buf[10];
	snprintf(buf, sizeof(buf), "%6gx", 10 * m_factor);
	return std::string(buf);
}

SystemView::SystemView() : UIView()
{
	SetTransparency(true);

	Graphics::RenderStateDesc rsd;
	m_lineState = Pi::renderer->CreateRenderState(rsd); //m_renderer not set yet

	m_realtime = true;

	Gui::Screen::PushFont("OverlayFont");
	m_objectLabels = new Gui::LabelSet();
	Add(m_objectLabels, 0, 0);
	Gui::Screen::PopFont();

	m_timePoint = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_timePoint, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);

	m_infoLabel = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_infoLabel, 2, 0);

	m_infoText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_infoText, 200, 0);

	m_zoomInButton = new Gui::ImageButton("icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	m_zoomInButton->SetRenderDimensions(30, 22);
	Add(m_zoomInButton, 700, 5);

	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	m_zoomOutButton->SetRenderDimensions(30, 22);
	Add(m_zoomOutButton, 732, 5);

	// orbital transfer planner UI
        int dx = 670;
	int dy = 40;

	m_plannerIncreaseFactorButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerIncreaseFactorButton->SetRenderDimensions(18, 18);
	m_plannerIncreaseFactorButton->onClick.connect(sigc::mem_fun(this, &SystemView::OnIncreaseFactorButtonClick));
	Add(m_plannerIncreaseFactorButton, dx + 40, dy);

	m_plannerResetFactorButton = new Gui::ImageButton("icons/orbit_factor_big.png");
	m_plannerResetFactorButton->SetRenderDimensions(18, 18);
	m_plannerResetFactorButton->SetToolTip(Lang::PLANNER_RESET_FACTOR);
	m_plannerResetFactorButton->onClick.connect(sigc::mem_fun(this, &SystemView::OnResetFactorButtonClick));
	Add(m_plannerResetFactorButton, dx + 20, dy);

	m_plannerDecreaseFactorButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerDecreaseFactorButton->SetRenderDimensions(18, 18);
	m_plannerDecreaseFactorButton->onClick.connect(sigc::mem_fun(this, &SystemView::OnDecreaseFactorButtonClick));
	Add(m_plannerDecreaseFactorButton, dx, dy);

	m_plannerFactorText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerFactorText, dx + 60 + 7, dy);

	m_plannerAddProgradeVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddProgradeVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddProgradeVelButton, dx + 40, dy + 20);

	m_plannerZeroProgradeVelButton = new Gui::ImageButton("icons/orbit_proretro_big.png");
	m_plannerZeroProgradeVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroProgradeVelButton->SetToolTip(Lang::PLANNER_RESET_PROGRADE);
	Add(m_plannerZeroProgradeVelButton, dx + 20, dy + 20);

	m_plannerAddRetrogradeVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddRetrogradeVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRetrogradeVelButton, dx, dy + 20);

	m_plannerProgradeDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerProgradeDvText, dx + 60, dy + 20);

	m_plannerAddNormalVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddNormalVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddNormalVelButton, dx + 40, dy + 40);

	m_plannerZeroNormalVelButton = new Gui::ImageButton("icons/orbit_normal_big.png");
	m_plannerZeroNormalVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroNormalVelButton->SetToolTip(Lang::PLANNER_RESET_NORMAL);
	Add(m_plannerZeroNormalVelButton, dx + 20, dy + 40);

	m_plannerAddAntiNormalVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddAntiNormalVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddAntiNormalVelButton, dx, dy + 40);

	m_plannerNormalDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerNormalDvText, dx + 60, dy + 40);

	m_plannerAddRadiallyInVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddRadiallyInVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRadiallyInVelButton, dx + 40, dy + 60);

	m_plannerZeroRadialVelButton = new Gui::ImageButton("icons/orbit_radial_big.png");
	m_plannerZeroRadialVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroRadialVelButton->SetToolTip(Lang::PLANNER_RESET_RADIAL);
	Add(m_plannerZeroRadialVelButton, dx + 20, dy + 60);

	m_plannerAddRadiallyOutVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddRadiallyOutVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRadiallyOutVelButton, dx, dy + 60);

	m_plannerRadialDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerRadialDvText, dx + 60, dy + 60);

	const int time_controls_left = Gui::Screen::GetWidth() - 150;
	const int time_controls_top = Gui::Screen::GetHeight() - 86;

	Gui::ImageButton *b = new Gui::ImageButton("icons/sysview_accel_r3.png", "icons/sysview_accel_r3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -10000000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(26, 17);
	Add(b, time_controls_left + 0, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_r2.png", "icons/sysview_accel_r2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -1000000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 26, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_r1.png", "icons/sysview_accel_r1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -100000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 45, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_rl.png", "icons/sysview_accel_rl_on.png");
	b->onPress.connect(sigc::mem_fun(this, &SystemView::OnClickRealt));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 64, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_f1.png", "icons/sysview_accel_f1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 100000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 83, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_f2.png", "icons/sysview_accel_f2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 1000000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 102, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_f3.png", "icons/sysview_accel_f3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 10000000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(26, 17);
	Add(b, time_controls_left + 121, time_controls_top);

	m_onMouseWheelCon =
		Pi::onMouseWheel.connect(sigc::mem_fun(this, &SystemView::MouseWheel));

	ResetViewpoint();

	m_planner = Pi::planner;
}

SystemView::~SystemView()
{
	m_onMouseWheelCon.disconnect();
}

void SystemView::OnClickAccel(float step)
{
	m_realtime = false;
	m_timeStep = step;
}

void SystemView::OnIncreaseFactorButtonClick(void) { m_planner->IncreaseFactor(); }
void SystemView::OnResetFactorButtonClick(void)    { m_planner->ResetFactor(); }
void SystemView::OnDecreaseFactorButtonClick(void) { m_planner->DecreaseFactor(); }

void SystemView::OnClickRealt()
{
	m_realtime = true;
}

void SystemView::ResetViewpoint()
{
	m_selectedObject = 0;
	m_rot_z = 0;
	m_rot_x = 50;
	m_zoom = 1.0f/float(AU);
	m_zoomTo = m_zoom;
	m_timeStep = 1.0f;
	m_time = Pi::game->GetTime();
}

void SystemView::PutOrbit(const Orbit *orbit, const vector3d &offset, const Color &color, double planetRadius)
{
	int num_vertices = 0;
	vector3f vts[100];
	for (int i = 0; i < int(COUNTOF(vts)); ++i) {
		const double t = double(i) / double(COUNTOF(vts));
		const vector3d pos = orbit->EvenSpacedPosTrajectory(t);
		vts[i] = vector3f(offset + pos * double(m_zoom));
		++num_vertices;
		if (pos.Length() < planetRadius)
			break;
	}

	if (num_vertices > 1) {
		// don't close the loop for hyperbolas and parabolas and crashed ellipses
		if ((orbit->GetEccentricity() > 1.0) || (num_vertices < int(COUNTOF(vts))))
			m_renderer->DrawLines(num_vertices, vts, color, m_lineState, LINE_STRIP);
		else
			m_renderer->DrawLines(num_vertices, vts, color, m_lineState, LINE_LOOP);
	}
}

void SystemView::OnClickObject(const SystemBody *b)
{
	m_selectedObject = b;
	std::string desc;
	std::string data;

	desc += std::string(Lang::NAME_OBJECT);
	desc += ":\n";
	data += b->GetName()+"\n";

	desc += std::string(Lang::DAY_LENGTH);
	desc += std::string(Lang::ROTATIONAL_PERIOD);
	desc += ":\n";
	data += stringf(Lang::N_DAYS, formatarg("days", b->GetRotationPeriodInDays())) + "\n";

	desc += std::string(Lang::RADIUS);
	desc += ":\n";
	data += format_distance(b->GetRadius())+"\n";

	if (b->GetParent()) {
		desc += std::string(Lang::SEMI_MAJOR_AXIS);
	desc += ":\n";
		data += format_distance(b->GetOrbit().GetSemiMajorAxis())+"\n";

		desc += std::string(Lang::ORBITAL_PERIOD);
	desc += ":\n";
		data += stringf(Lang::N_DAYS, formatarg("days", b->GetOrbit().Period() / (24*60*60))) + "\n";
	}
	m_infoLabel->SetText(desc);
	m_infoText->SetText(data);

	if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
		SystemPath path = m_system->GetPathOf(b);
		if (Pi::game->GetSpace()->GetStarSystem()->GetPath() == m_system->GetPath()) {
			Body* body = Pi::game->GetSpace()->FindBodyForPath(&path);
			if (body != 0)
				Pi::player->SetNavTarget(body);
		}
	}
}

void SystemView::PutLabel(const SystemBody *b, const vector3d &offset)
{
	Gui::Screen::EnterOrtho();

	vector3d pos;
	if (Gui::Screen::Project(offset, pos)) {
		// libsigc++ is a beautiful thing
		m_objectLabels->Add(b->GetName(), sigc::bind(sigc::mem_fun(this, &SystemView::OnClickObject), b), pos.x, pos.y);
	}

	Gui::Screen::LeaveOrtho();
}

void SystemView::PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans)
{
	if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE) return;
	if (b->GetType() != SystemBody::TYPE_GRAVPOINT) {

		if (!m_bodyIcon) {
			Graphics::RenderStateDesc rsd;
			auto solidState = m_renderer->CreateRenderState(rsd);
			m_bodyIcon.reset(new Graphics::Drawables::Disk(m_renderer, solidState, Color::WHITE, 1.0f));
		}

		const double radius = b->GetRadius() * m_zoom;

		matrix4x4f invRot = trans;
		invRot.ClearToRotOnly();
		invRot = invRot.InverseOf();

		matrix4x4f bodyTrans = trans;
		bodyTrans.Translate(vector3f(offset));
		bodyTrans.Scale(radius);
		m_renderer->SetTransform(bodyTrans * invRot);
		m_bodyIcon->Draw(m_renderer);

		m_renderer->SetTransform(trans);

		PutLabel(b, offset);
	}

	Frame *frame = Pi::player->GetFrame();
	if(frame->IsRotFrame()) frame = frame->GetNonRotFrame();
	if(frame->GetSystemBody() == b && frame->GetSystemBody()->GetMass() > 0) {
		const double t0 = Pi::game->GetTime();
		Orbit playerOrbit = Pi::player->ComputeOrbit();

		Orbit plannedOrbit = Orbit::FromBodyState(Pi::player->GetPosition(),
		//Orbit plannedOrbit = Orbit::FromBodyState(playerOrbit.OrbitalPosAtTime(m_time - t0),
							  m_planner->GetVel(),
							  frame->GetSystemBody()->GetMass());

		PutOrbit(&playerOrbit, offset, Color::RED, b->GetRadius());
		PutOrbit(&plannedOrbit, offset, Color::STEELBLUE, b->GetRadius());
		PutSelectionBox(offset + playerOrbit.OrbitalPosAtTime(m_time - t0)* double(m_zoom), Color::RED);
	}

	if (b->HasChildren()) {
		for(const SystemBody* kid : b->GetChildren()) {
			if (is_zero_general(kid->GetOrbit().GetSemiMajorAxis())) continue;
			if (kid->GetOrbit().GetSemiMajorAxis() * m_zoom < ROUGH_SIZE_OF_TURD) {
				PutOrbit(&(kid->GetOrbit()), offset, Color(0, 255, 0, 255));
			}

			// not using current time yet
			vector3d pos = kid->GetOrbit().OrbitalPosAtTime(m_time);
			pos *= double(m_zoom);

			PutBody(kid, offset + pos, trans);
		}
	}
}

void SystemView::PutSelectionBox(const SystemBody *b, const vector3d &rootPos, const Color &col)
{
	// surface starports just show the planet as being selected,
	// because SystemView doesn't render terrains anyway
	if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
		b = b->GetParent();
	assert(b);

	vector3d pos = rootPos;
	// while (b->parent), not while (b) because the root SystemBody is defined to be at (0,0,0)
	while (b->GetParent()) {
		pos += b->GetOrbit().OrbitalPosAtTime(m_time) * double(m_zoom);
		b = b->GetParent();
	}

	PutSelectionBox(pos, col);
}

void SystemView::PutSelectionBox(const vector3d &worldPos, const Color &col)
{
	Gui::Screen::EnterOrtho();

	vector3d screenPos;
	if (Gui::Screen::Project(worldPos, screenPos)) {
		// XXX copied from WorldView::DrawTargetSquare -- these should be unified
		const float x1 = float(screenPos.x - SystemView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float x2 = float(x1 + SystemView::PICK_OBJECT_RECT_SIZE);
		const float y1 = float(screenPos.y - SystemView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float y2 = float(y1 + SystemView::PICK_OBJECT_RECT_SIZE);

        const vector3f verts[4] = {
                vector3f(x1, y1, 0.f),
                vector3f(x2, y1, 0.f),
                vector3f(x2, y2, 0.f),
                vector3f(x1, y2, 0.f)
        };
		m_renderer->DrawLines(4, &verts[0], col, m_lineState, Graphics::LINE_LOOP);
	}

	Gui::Screen::LeaveOrtho();
}

static const GLfloat fogDensity = 0.1f;
static const GLfloat fogColor[4] = { 0,0,0,1.0f };

void SystemView::GetTransformTo(const SystemBody *b, vector3d &pos)
{
	if (b->GetParent()) {
		GetTransformTo(b->GetParent(), pos);
		pos -= double(m_zoom) * b->GetOrbit().OrbitalPosAtTime(m_time);
	}
}

void SystemView::Draw3D()
{
	PROFILE_SCOPED()
	m_renderer->SetPerspectiveProjection(50.f, m_renderer->GetDisplayAspect(), 1.f, 1000.f);
	m_renderer->ClearScreen();

	SystemPath path = Pi::sectorView->GetSelected().SystemOnly();
	if (m_system) {
		if (!m_system->GetPath().IsSameSystem(path)) {
			m_system.Reset();
			ResetViewpoint();
		}
	}

	if (m_realtime) {
		m_time = Pi::game->GetTime();
	}
	else {
		m_time += m_timeStep*Pi::GetFrameTime();
	}
	std::string t = Lang::TIME_POINT+format_date(m_time);
	m_timePoint->SetText(t);

	if (!m_system) m_system = Pi::GetGalaxy()->GetStarSystem(path);

	matrix4x4f trans = matrix4x4f::Identity();
	trans.Translate(0,0,-ROUGH_SIZE_OF_TURD);
	trans.Rotate(DEG2RAD(m_rot_x), 1, 0, 0);
	trans.Rotate(DEG2RAD(m_rot_z), 0, 0, 1);
	m_renderer->SetTransform(trans);

	vector3d pos(0,0,0);
	if (m_selectedObject) GetTransformTo(m_selectedObject, pos);

	m_objectLabels->Clear();
	if (m_system->GetUnexplored())
		m_infoLabel->SetText(Lang::UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW);
	else if (m_system->GetRootBody()) {
		PutBody(m_system->GetRootBody().Get(), pos, trans);
		if (Pi::game->GetSpace()->GetStarSystem() == m_system) {
			const Body *navTarget = Pi::player->GetNavTarget();
			const SystemBody *navTargetSystemBody = navTarget ? navTarget->GetSystemBody() : 0;
			if (navTargetSystemBody)
				PutSelectionBox(navTargetSystemBody, pos, Color::GREEN);
		}
	}

	UIView::Draw3D();
}

void SystemView::Update()
{
	const float ft = Pi::GetFrameTime();
	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
		if (Pi::KeyState(SDLK_EQUALS) ||
			m_zoomInButton->IsPressed())
				m_zoomTo *= pow(ZOOM_IN_SPEED * Pi::GetMoveSpeedShiftModifier(), ft);
		if (Pi::KeyState(SDLK_MINUS) ||
			m_zoomOutButton->IsPressed())
				m_zoomTo *= pow(ZOOM_OUT_SPEED / Pi::GetMoveSpeedShiftModifier(), ft);

		// transfer planner buttons
		if (m_plannerAddProgradeVelButton->IsPressed())    { m_planner->AddDv(PROGRADE,  10.0); }
		if (m_plannerAddRetrogradeVelButton->IsPressed())  { m_planner->AddDv(PROGRADE, -10.0); }
		if (m_plannerAddNormalVelButton->IsPressed())      { m_planner->AddDv(NORMAL,    10.0); }
		if (m_plannerAddAntiNormalVelButton->IsPressed())  { m_planner->AddDv(NORMAL,   -10.0); }
		if (m_plannerAddRadiallyInVelButton->IsPressed())  { m_planner->AddDv(RADIAL,    10.0); }
		if (m_plannerAddRadiallyOutVelButton->IsPressed()) { m_planner->AddDv(RADIAL,   -10.0); }
		if (m_plannerZeroProgradeVelButton->IsPressed())   { m_planner->ResetDv(PROGRADE); }
		if (m_plannerZeroNormalVelButton->IsPressed())     { m_planner->ResetDv(NORMAL);   }
		if (m_plannerZeroRadialVelButton->IsPressed())     { m_planner->ResetDv(RADIAL);   }

		m_plannerFactorText->SetText(m_planner->printFactor());
		m_plannerProgradeDvText->SetText(m_planner->printDv(PROGRADE));
		m_plannerNormalDvText->SetText(m_planner->printDv(NORMAL));
		m_plannerRadialDvText->SetText(m_planner->printDv(RADIAL));

	}
	// TODO: add "true" lower/upper bounds to m_zoomTo / m_zoom
	m_zoomTo = Clamp(m_zoomTo, MIN_ZOOM, MAX_ZOOM);
	m_zoom = Clamp(m_zoom, MIN_ZOOM, MAX_ZOOM);
	AnimationCurves::Approach(m_zoom, m_zoomTo, ft);

	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1]*20*ft;
		m_rot_z += motion[0]*20*ft;
	}

	UIView::Update();
}

void SystemView::MouseWheel(bool up)
{
	if (this == Pi::GetView()) {
		if (!up)
			m_zoomTo *= ((ZOOM_OUT_SPEED-1) * WHEEL_SENSITIVITY+1) / Pi::GetMoveSpeedShiftModifier();
		else
			m_zoomTo *= ((ZOOM_IN_SPEED-1) * WHEEL_SENSITIVITY+1) * Pi::GetMoveSpeedShiftModifier();
	}
}
