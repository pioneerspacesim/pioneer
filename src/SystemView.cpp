// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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
#include "MathUtil.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include <sstream>
#include <iomanip>

using namespace Graphics;

const double SystemView::PICK_OBJECT_RECT_SIZE = 12.0;
const Uint16 SystemView::N_VERTICES_MAX = 100;
static const float MIN_ZOOM = 1e-30f;		// Just to avoid having 0
static const float MAX_ZOOM = 1e30f;
static const float ZOOM_IN_SPEED = 2;
static const float ZOOM_OUT_SPEED = 1.f/ZOOM_IN_SPEED;
static const float WHEEL_SENSITIVITY = .1f;		// Should be a variable in user settings.
static const double DEFAULT_VIEW_DISTANCE = 10.0;

TransferPlanner::TransferPlanner() :
	m_position(0., 0., 0.), m_velocity(0., 0., 0.)
{
	m_dvPrograde = 0.0;
	m_dvNormal = 0.0;
	m_dvRadial = 0.0;
	m_startTime = 0.0;
	m_factor = 1;
}

vector3d TransferPlanner::GetVel() const { return m_velocity + GetOffsetVel(); }

vector3d TransferPlanner::GetOffsetVel() const {
	if(m_position.ExactlyEqual(vector3d(0., 0., 0.)))
		return vector3d(0., 0., 0.);

	const vector3d pNormal = m_position.Cross(m_velocity);

	return m_dvPrograde * m_velocity.Normalized() +
		   m_dvNormal   * pNormal.Normalized() +
		   m_dvRadial   * m_position.Normalized();
}

void TransferPlanner::AddStartTime(double timeStep) {
	if(std::fabs(m_startTime) < 1.)
		m_startTime = Pi::game->GetTime();

	m_startTime += m_factor * timeStep;
	double deltaT = m_startTime - Pi::game->GetTime();
	if(deltaT > 0.)
	{
		Frame *frame = Pi::player->GetFrame()->GetNonRotFrame();
		Orbit playerOrbit = Orbit::FromBodyState(Pi::player->GetPositionRelTo(frame), Pi::player->GetVelocityRelTo(frame), frame->GetSystemBody()->GetMass());

		m_position = playerOrbit.OrbitalPosAtTime(deltaT);
		m_velocity = playerOrbit.OrbitalVelocityAtTime(frame->GetSystemBody()->GetMass(), deltaT);
	}
	else
		ResetStartTime();
}

void TransferPlanner::ResetStartTime() {
	m_startTime = 0;
	Frame *frame = Pi::player->GetFrame();
	if(!frame || GetOffsetVel().ExactlyEqual(vector3d(0., 0. , 0.)))
	{
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0. , 0., 0.);
	}
	else
	{
		frame = frame->GetNonRotFrame();
		m_position = Pi::player->GetPositionRelTo(frame);
		m_velocity = Pi::player->GetVelocityRelTo(frame);
	}
}

double TransferPlanner::GetStartTime() const {
	return m_startTime;
}

static std::string formatTime(double t)
{
	std::stringstream formattedTime;
	formattedTime << std::setprecision(1) << std::fixed;
	double absT = fabs(t);
	if(absT < 60.)
		formattedTime << t << "s";
	else if(absT < 3600)
		formattedTime << t / 60. << "m";
	else if(absT < 86400)
		formattedTime << t / 3600. << "h";
	else if(absT < 31536000)
		formattedTime << t / 86400. << "d";
	else
		formattedTime << t / 31536000. << "y";
	return formattedTime.str();
}

std::string TransferPlanner::printDeltaTime() {
	std::stringstream out;
	out << std::setw(9);
	double deltaT = m_startTime - Pi::game->GetTime();
	if(std::fabs(m_startTime) < 1.)
		out << Lang::NOW;
	else
		out << formatTime(deltaT);

	return out.str();
}

void TransferPlanner::AddDv(BurnDirection d, double dv) {
	if(m_position.ExactlyEqual(vector3d(0., 0., 0.)))
	{
		Frame *frame = Pi::player->GetFrame()->GetNonRotFrame();
		m_position = Pi::player->GetPositionRelTo(frame);
		m_velocity = Pi::player->GetVelocityRelTo(frame);
		m_startTime = Pi::game->GetTime();
	}

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

	if(std::fabs(m_startTime) < 1. &&
	   GetOffsetVel().ExactlyEqual(vector3d(0., 0., 0.)))
	{
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0., 0., 0.);
		m_startTime = 0.;
	}
}

void TransferPlanner::ResetDv() {
    m_dvPrograde = 0;
    m_dvNormal = 0;
    m_dvRadial = 0;

    if(std::fabs(m_startTime) < 1.) {
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0., 0., 0.);
		m_startTime = 0.;
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
	char buf[16];
	snprintf(buf, sizeof(buf), "%8gx", 10 * m_factor);
	return std::string(buf);
}

vector3d TransferPlanner::GetPosition() const { return m_position; }

void TransferPlanner::SetPosition(const vector3d& position) { m_position = position; }

SystemView::SystemView(Game* game) : UIView(), m_game(game)
{
	SetTransparency(true);

	Graphics::RenderStateDesc rsd;
	m_lineState = Pi::renderer->CreateRenderState(rsd); //m_renderer not set yet

	m_realtime = true;
	m_unexplored = true;

	Gui::Screen::PushFont("OverlayFont");
	m_objectLabels = new Gui::LabelSet();
	Add(m_objectLabels, 0, 0);
	m_shipLabels = new Gui::LabelSet();
	Add(m_shipLabels, 0, 0);
	Gui::Screen::PopFont();

	m_timePoint = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_timePoint, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);

	m_infoLabel = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_infoLabel, 2, 0);

	m_infoText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_infoText, 200, 0);

	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	m_zoomOutButton->SetRenderDimensions(30, 22);
	Add(m_zoomOutButton, 700, 5);

	m_zoomInButton = new Gui::ImageButton("icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	m_zoomInButton->SetRenderDimensions(30, 22);
	Add(m_zoomInButton, 732, 5);

	m_toggleShipsButton = new Gui::ImageButton("icons/toggle_ships_display.png");
	m_toggleShipsButton->SetToolTip(Lang::SHIPS_DISPLAY_MODE_TOGGLE);
	m_toggleShipsButton->SetRenderDimensions(30, 22);
	m_toggleShipsButton->onClick.connect(sigc::mem_fun(this, &SystemView::OnToggleShipsButtonClick));
	Add(m_toggleShipsButton, 660, 5);

	// Add the 3 Lagrange button stations
	m_toggleL4L5Button = new Gui::MultiStateImageButton();
	m_toggleL4L5Button->AddState(LAG_ICON,		"icons/toggle_lag_icon.png");
	m_toggleL4L5Button->AddState(LAG_ICONTEXT,	"icons/toggle_lag_icon_text.png");
	m_toggleL4L5Button->AddState(LAG_OFF,		"icons/toggle_lag_off.png");
	m_toggleL4L5Button->SetToolTip(Lang::L4L5_DISPLAY_MODE_TOGGLE);
	m_toggleL4L5Button->SetRenderDimensions(30, 22);
	m_toggleL4L5Button->onClick.connect(sigc::mem_fun(this, &SystemView::OnToggleL4L5ButtonClick));
	Add(m_toggleL4L5Button, 628, 5);
	m_toggleL4L5Button->SetActiveState(LAG_OFF);

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

	m_plannerIncreaseStartTimeButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerIncreaseStartTimeButton->SetRenderDimensions(18, 18);
	Add(m_plannerIncreaseStartTimeButton, dx + 40, dy + 20);

	m_plannerResetStartTimeButton = new Gui::ImageButton("icons/orbit_start_big.png");
	m_plannerResetStartTimeButton->SetRenderDimensions(18, 18);
	m_plannerResetStartTimeButton->SetToolTip(Lang::PLANNER_RESET_START);
	Add(m_plannerResetStartTimeButton, dx + 20, dy + 20);

	m_plannerDecreaseStartTimeButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerDecreaseStartTimeButton->SetRenderDimensions(18, 18);
	Add(m_plannerDecreaseStartTimeButton, dx, dy + 20);

	m_plannerStartTimeText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerStartTimeText, dx + 60, dy + 20);

	m_plannerAddProgradeVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddProgradeVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddProgradeVelButton, dx + 40, dy + 40);

	m_plannerZeroProgradeVelButton = new Gui::ImageButton("icons/orbit_proretro_big.png");
	m_plannerZeroProgradeVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroProgradeVelButton->SetToolTip(Lang::PLANNER_RESET_PROGRADE);
	Add(m_plannerZeroProgradeVelButton, dx + 20, dy + 40);

	m_plannerAddRetrogradeVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddRetrogradeVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRetrogradeVelButton, dx, dy + 40);

	m_plannerProgradeDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerProgradeDvText, dx + 60, dy + 40);

	m_plannerAddNormalVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddNormalVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddNormalVelButton, dx + 40, dy + 60);

	m_plannerZeroNormalVelButton = new Gui::ImageButton("icons/orbit_normal_big.png");
	m_plannerZeroNormalVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroNormalVelButton->SetToolTip(Lang::PLANNER_RESET_NORMAL);
	Add(m_plannerZeroNormalVelButton, dx + 20, dy + 60);

	m_plannerAddAntiNormalVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddAntiNormalVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddAntiNormalVelButton, dx, dy + 60);

	m_plannerNormalDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerNormalDvText, dx + 60, dy + 60);

	m_plannerAddRadiallyInVelButton = new Gui::ImageButton("icons/orbit_increase_big.png");
	m_plannerAddRadiallyInVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRadiallyInVelButton, dx + 40, dy + 80);

	m_plannerZeroRadialVelButton = new Gui::ImageButton("icons/orbit_radial_big.png");
	m_plannerZeroRadialVelButton->SetRenderDimensions(18, 18);
	m_plannerZeroRadialVelButton->SetToolTip(Lang::PLANNER_RESET_RADIAL);
	Add(m_plannerZeroRadialVelButton, dx + 20, dy + 80);

	m_plannerAddRadiallyOutVelButton = new Gui::ImageButton("icons/orbit_reduce_big.png");
	m_plannerAddRadiallyOutVelButton->SetRenderDimensions(18, 18);
	Add(m_plannerAddRadiallyOutVelButton, dx, dy + 80);

	m_plannerRadialDvText = (new Gui::Label(""))->Color(178, 178, 178);
	Add(m_plannerRadialDvText, dx + 60, dy + 80);

	const int time_controls_left = Gui::Screen::GetWidth() - 150;
	const int time_controls_top = Gui::Screen::GetHeight() - 86;

	Gui::ImageButton *b = new Gui::ImageButton("icons/sysview_accel_r3.png", "icons/sysview_accel_r3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -10000000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(26, 17);
	Add(b, time_controls_left + 0, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_r2.png", "icons/sysview_accel_r2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -100000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 26, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_r1.png", "icons/sysview_accel_r1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -1000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 45, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_rl.png", "icons/sysview_accel_rl_on.png");
	b->onPress.connect(sigc::mem_fun(this, &SystemView::OnClickRealt));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 64, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_f1.png", "icons/sysview_accel_f1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 1000.f));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0f));
	b->SetRenderDimensions(19, 17);
	Add(b, time_controls_left + 83, time_controls_top);

	b = new Gui::ImageButton("icons/sysview_accel_f2.png", "icons/sysview_accel_f2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 100000.f));
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

	Graphics::TextureBuilder b1 = Graphics::TextureBuilder::UI("icons/periapsis.png");
	m_periapsisIcon.reset(new Gui::TexturedQuad(b1.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));
	Graphics::TextureBuilder b2 = Graphics::TextureBuilder::UI("icons/apoapsis.png");
	m_apoapsisIcon.reset(new Gui::TexturedQuad(b2.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));

	Graphics::TextureBuilder l4 = Graphics::TextureBuilder::UI("icons/l4.png");
	m_l4Icon.reset(new Gui::TexturedQuad(l4.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));
	Graphics::TextureBuilder l5 = Graphics::TextureBuilder::UI("icons/l5.png");
	m_l5Icon.reset(new Gui::TexturedQuad(l5.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));

	ResetViewpoint();

	RefreshShips();
	m_shipDrawing = OFF;
	m_showL4L5 = LAG_OFF;
	m_planner = Pi::planner;

	m_orbitVts.reset( new vector3f[N_VERTICES_MAX] );
	m_orbitColors.reset( new Color[N_VERTICES_MAX] );
}

SystemView::~SystemView()
{
	m_contacts.clear();
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

void SystemView::OnToggleShipsButtonClick(void) {
	switch(m_shipDrawing) {
	case OFF:    m_shipDrawing = BOXES;  RefreshShips(); break;
	case BOXES:  m_shipDrawing = ORBITS; RefreshShips(); break;
	case ORBITS: m_shipDrawing = OFF; m_shipLabels->Clear(); break;
	}
}

void SystemView::OnToggleL4L5ButtonClick(Gui::MultiStateImageButton *b) {
	switch (m_showL4L5)
	{
	case LAG_OFF:		m_showL4L5 = LAG_ICON;		m_toggleL4L5Button->SetActiveState(LAG_ICON);		break;
	case LAG_ICON:		m_showL4L5 = LAG_ICONTEXT;	m_toggleL4L5Button->SetActiveState(LAG_ICONTEXT);	break;
	case LAG_ICONTEXT:	m_showL4L5 = LAG_OFF;		m_toggleL4L5Button->SetActiveState(LAG_OFF);		break;
	}
}

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
	m_time = m_game->GetTime();
}

void SystemView::PutOrbit(const Orbit *orbit, const vector3d &offset, const Color &color, const double planetRadius, const bool showLagrange)
{
	double maxT = 1.;
	unsigned short num_vertices = 0;
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = double(i) / double(N_VERTICES_MAX);
		const vector3d pos = orbit->EvenSpacedPosTrajectory(t);
		if (pos.Length() < planetRadius)
		{
			maxT = t;
			break;
		}
	}

	static const float startTrailPercent = 0.85;
	static const float fadedColorParameter = 0.1;

	Uint16 fadingColors = 0;
	const double tMinust0 = m_time - m_game->GetTime();
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = double(i) / double(N_VERTICES_MAX) * maxT;
		if(fadingColors == 0 && t >= startTrailPercent * maxT)
			fadingColors = i;
		const vector3d pos = orbit->EvenSpacedPosTrajectory(t, tMinust0);
		m_orbitVts[i] = vector3f(offset + pos * double(m_zoom));
		++num_vertices;
		if (pos.Length() < planetRadius)
			break;
	}

	const Color fadedColor = color * fadedColorParameter;
	std::fill_n(m_orbitColors.get(), num_vertices, fadedColor);
	const Uint16 trailLength = num_vertices - fadingColors;

	for (Uint16 currentColor = 0; currentColor < trailLength; ++currentColor)
	{
		float scalingParameter = fadedColorParameter + static_cast<float>(currentColor) / trailLength * (1.f - fadedColorParameter);
		m_orbitColors[currentColor + fadingColors] = color * scalingParameter;
	}

	if (num_vertices > 1) {
		m_orbits.SetData(num_vertices, m_orbitVts.get(), m_orbitColors.get());

		// don't close the loop for hyperbolas and parabolas and crashed ellipses
		if (maxT < 1. || (orbit->GetEccentricity() > 1.0)) {
			m_orbits.Draw(m_renderer, m_lineState, LINE_STRIP);
		} else {
			m_orbits.Draw(m_renderer, m_lineState, LINE_LOOP);
		}
	}

	Gui::Screen::EnterOrtho();
	vector3d pos;
	if(Gui::Screen::Project(offset + orbit->Perigeum() * double(m_zoom), pos))
		m_periapsisIcon->Draw(Pi::renderer, vector2f(pos.x-3, pos.y-5), vector2f(6,10), color);
	if(Gui::Screen::Project(offset + orbit->Apogeum() * double(m_zoom), pos))
		m_apoapsisIcon->Draw(Pi::renderer, vector2f(pos.x-3, pos.y-5), vector2f(6,10), color);

	if (showLagrange && m_showL4L5!=LAG_OFF)
	{
		const Color LPointColor(0x00d6e2ff);
		const vector3d posL4 = orbit->EvenSpacedPosTrajectory((1.0 / 360.0) * 60.0, tMinust0);
		if (Gui::Screen::Project(offset + posL4 * double(m_zoom), pos)) {
			m_l4Icon->Draw(Pi::renderer, vector2f(pos.x - 2, pos.y - 2), vector2f(4, 4), LPointColor);
			if(m_showL4L5==LAG_ICONTEXT)
				m_objectLabels->Add(std::string("L4"), sigc::mem_fun(this, &SystemView::OnClickLagrange), pos.x, pos.y);
		}

		const vector3d posL5 = orbit->EvenSpacedPosTrajectory((1.0 / 360.0) * 300.0, tMinust0);
		if (Gui::Screen::Project(offset + posL5 * double(m_zoom), pos)) {
			m_l5Icon->Draw(Pi::renderer, vector2f(pos.x - 2, pos.y - 2), vector2f(4, 4), LPointColor);
			if (m_showL4L5 == LAG_ICONTEXT)
				m_objectLabels->Add(std::string("L5"), sigc::mem_fun(this, &SystemView::OnClickLagrange), pos.x, pos.y);
		}
	}
	Gui::Screen::LeaveOrtho();
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

	// click on object (in same system) sets/unsets it as nav target
	SystemPath path = m_system->GetPathOf(b);
	if (m_game->GetSpace()->GetStarSystem()->GetPath() == m_system->GetPath()) {
		Body* body = m_game->GetSpace()->FindBodyForPath(&path);
		if (body != 0) {
			if(Pi::player->GetNavTarget() == body) {
				Pi::player->SetNavTarget(body);
				Pi::player->SetNavTarget(0);
				m_game->log->Add(Lang::UNSET_NAVTARGET);
			}
			else {
				Pi::player->SetNavTarget(body);
				m_game->log->Add(Lang::SET_NAVTARGET_TO + body->GetLabel());
			}
		}
	}
}

void SystemView::OnClickLagrange()
{

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

void SystemView::LabelShip(Ship *s, const vector3d &offset) {
	Gui::Screen::EnterOrtho();

	vector3d pos;
	if (Gui::Screen::Project(offset, pos)) {
		m_shipLabels->Add(s->GetLabel(), sigc::bind(sigc::mem_fun(this, &SystemView::OnClickShip), s), pos.x, pos.y);
	}

	Gui::Screen::LeaveOrtho();
}

void SystemView::OnClickShip(Ship *s) {
	if(!s) { printf("clicked on ship label but ship wasn't there\n"); return; }
	if(Pi::player->GetNavTarget() == s) { //un-select ship if already selected
		Pi::player->SetNavTarget(0); // remove current
		m_game->log->Add(Lang::UNSET_NAVTARGET);
		m_infoLabel->SetText("");    // remove lingering text
		m_infoText->SetText("");
	} else {
		Pi::player->SetNavTarget(s);
		m_game->log->Add(Lang::SET_NAVTARGET_TO + s->GetLabel());

		// always show label of selected ship...
		std::string text;
		text += s->GetLabel();
		text += "\n";

		// ...if we have advanced target scanner equipment, show some extra info on selected ship
		int prop_var = 0;
		Pi::player->Properties().Get("target_scanner_level_cap", prop_var);
		if (prop_var > 1) {  // advanced target scanner
			const shipstats_t &stats = s->GetStats();

			text += s->GetShipType()->name;
			text += "\n";

			lua_State * l = Lua::manager->GetLuaState();
			int clean_stack = lua_gettop(l);

			LuaObject<Ship>::CallMethod<LuaRef>(s, "GetEquip", "engine").PushCopyToStack();
			lua_rawgeti(l, -1, 1);
			if (lua_isnil(l, -1)) {
				text += Lang::NO_HYPERDRIVE;
			} else {
				text += LuaTable(l, -1).CallMethod<std::string>("GetName");
			}
			lua_settop(l, clean_stack);

			text += "\n";
			text += stringf(Lang::MASS_N_TONNES, formatarg("mass", stats.static_mass));
			text += "\n";
			text += stringf(Lang::CARGO_N, formatarg("mass", stats.used_cargo));
			text += "\n";
		}

		m_infoLabel->SetText(text);
		m_infoText->SetText("");  // clear lingering info from body being selected

	}
}

void SystemView::PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans)
{
	if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
		return;

	if (b->GetType() != SystemBody::TYPE_GRAVPOINT)
	{
		if (!m_bodyIcon)
		{
			Graphics::RenderStateDesc rsd;
			auto solidState = m_renderer->CreateRenderState(rsd);
			m_bodyIcon.reset(new Graphics::Drawables::Disk(m_renderer, solidState, Color::WHITE, 1.0f));
		}

		const double radius = b->GetRadius() * m_zoom;

		matrix4x4f invRot = trans;
		invRot.ClearToRotOnly();
		invRot = invRot.Inverse();

		matrix4x4f bodyTrans = trans;
		bodyTrans.Translate(vector3f(offset));
		bodyTrans.Scale(radius);
		m_renderer->SetTransform(bodyTrans * invRot);
		m_bodyIcon->Draw(m_renderer);

		m_renderer->SetTransform(trans);

		PutLabel(b, offset);
	}

	Frame *frame = Pi::player->GetFrame();
	if(frame->IsRotFrame())
		frame = frame->GetNonRotFrame();

	// display the players orbit(?)
	if(frame->GetSystemBody() == b && frame->GetSystemBody()->GetMass() > 0)
	{
		const double t0 = m_game->GetTime();
		Orbit playerOrbit = Pi::player->ComputeOrbit();

		PutOrbit(&playerOrbit, offset, Color::RED, b->GetRadius());

		const double plannerStartTime = m_planner->GetStartTime();
		if(!m_planner->GetPosition().ExactlyEqual(vector3d(0,0,0)))
		{
			Orbit plannedOrbit = Orbit::FromBodyState(m_planner->GetPosition(),
								  m_planner->GetVel(),
								  frame->GetSystemBody()->GetMass());
			PutOrbit(&plannedOrbit, offset, Color::STEELBLUE, b->GetRadius());
			if(std::fabs(m_time - t0) > 1. && (m_time - plannerStartTime) > 0.)
				PutSelectionBox(offset + plannedOrbit.OrbitalPosAtTime(m_time - plannerStartTime) * static_cast<double>(m_zoom), Color::STEELBLUE);
			else
				PutSelectionBox(offset + m_planner->GetPosition() * static_cast<double>(m_zoom), Color::STEELBLUE);

		}

		PutSelectionBox(offset + playerOrbit.OrbitalPosAtTime(m_time - t0)* double(m_zoom), Color::RED);
	}

	// display all child bodies and their orbits
	if (b->HasChildren())
	{
		for(const SystemBody* kid : b->GetChildren())
		{
			if (is_zero_general(kid->GetOrbit().GetSemiMajorAxis()))
				continue;

			const double axisZoom = kid->GetOrbit().GetSemiMajorAxis() * m_zoom;
			if (axisZoom < DEFAULT_VIEW_DISTANCE)
			{
				const SystemBody::BodySuperType bst = kid->GetSuperType();
				const bool showLagrange = (bst == SystemBody::SUPERTYPE_ROCKY_PLANET || bst == SystemBody::SUPERTYPE_GAS_GIANT);
				PutOrbit(&(kid->GetOrbit()), offset, Color::GREEN, 0.0, showLagrange);
			}

			// not using current time yet
			const vector3d pos = kid->GetOrbit().OrbitalPosAtTime(m_time) * double(m_zoom);
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
		m_selectBox.SetData(4, &verts[0], col);
		m_selectBox.Draw(m_renderer, m_lineState, Graphics::LINE_LOOP);
	}

	Gui::Screen::LeaveOrtho();
}

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

	SystemPath path = m_game->GetSectorView()->GetSelected().SystemOnly();
	if (m_system) {
		if (m_system->GetUnexplored() != m_unexplored || !m_system->GetPath().IsSameSystem(path)) {
			m_system.Reset();
			ResetViewpoint();
		}
	}

	if (m_realtime) {
		m_time = m_game->GetTime();
	}
	else {
		m_time += m_timeStep*Pi::GetFrameTime();
	}
	std::string t = Lang::TIME_POINT+format_date(m_time);
	m_timePoint->SetText(t);

	if (!m_system) {
		m_system = m_game->GetGalaxy()->GetStarSystem(path);
		m_unexplored = m_system->GetUnexplored();
	}

	matrix4x4f trans = matrix4x4f::Identity();
	trans.Translate(0,0,-DEFAULT_VIEW_DISTANCE);
	trans.Rotate(DEG2RAD(m_rot_x), 1, 0, 0);
	trans.Rotate(DEG2RAD(m_rot_z), 0, 0, 1);
	m_renderer->SetTransform(trans);

	vector3d pos(0,0,0);
	if (m_selectedObject) GetTransformTo(m_selectedObject, pos);

	// glLineWidth(2);
	m_objectLabels->Clear();
	if (m_system->GetUnexplored())
		m_infoLabel->SetText(Lang::UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW);
	else {
		m_infoLabel->SetText("");
		if (m_system->GetRootBody()) {
			PutBody(m_system->GetRootBody().Get(), pos, trans);
			if (m_game->GetSpace()->GetStarSystem() == m_system) {
				const Body *navTarget = Pi::player->GetNavTarget();
				const SystemBody *navTargetSystemBody = navTarget ? navTarget->GetSystemBody() : 0;
				if (navTargetSystemBody)
					PutSelectionBox(navTargetSystemBody, pos, Color::GREEN);
			}
		}
	}
	// glLineWidth(1);

	if(m_shipDrawing != OFF) {
		RefreshShips();
		DrawShips(m_time - m_game->GetTime(), pos);
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
		if (m_plannerIncreaseStartTimeButton->IsPressed()) { m_planner->AddStartTime( 10.); }
		if (m_plannerDecreaseStartTimeButton->IsPressed()) { m_planner->AddStartTime(-10.); }
		if (m_plannerAddProgradeVelButton->IsPressed())    { m_planner->AddDv(PROGRADE,  10.0); }
		if (m_plannerAddRetrogradeVelButton->IsPressed())  { m_planner->AddDv(PROGRADE, -10.0); }
		if (m_plannerAddNormalVelButton->IsPressed())      { m_planner->AddDv(NORMAL,    10.0); }
		if (m_plannerAddAntiNormalVelButton->IsPressed())  { m_planner->AddDv(NORMAL,   -10.0); }
		if (m_plannerAddRadiallyInVelButton->IsPressed())  { m_planner->AddDv(RADIAL,    10.0); }
		if (m_plannerAddRadiallyOutVelButton->IsPressed()) { m_planner->AddDv(RADIAL,   -10.0); }
		if (m_plannerResetStartTimeButton->IsPressed())	   { m_planner->ResetStartTime();  }
		if (m_plannerZeroProgradeVelButton->IsPressed())   { m_planner->ResetDv(PROGRADE); }
		if (m_plannerZeroNormalVelButton->IsPressed())     { m_planner->ResetDv(NORMAL);   }
		if (m_plannerZeroRadialVelButton->IsPressed())     { m_planner->ResetDv(RADIAL);   }

		m_plannerFactorText->SetText(m_planner->printFactor());
		m_plannerStartTimeText->SetText(m_planner->printDeltaTime());
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

void SystemView::RefreshShips(void) {
	m_contacts.clear();
	if(!m_game->GetSpace()->GetStarSystem()->GetPath().IsSameSystem(m_game->GetSectorView()->GetSelected()))
		return;

	auto bs = m_game->GetSpace()->GetBodies();
	for(auto s = bs.begin(); s != bs.end(); s++) {
		if((*s) != Pi::player &&
		   (*s)->GetType() == Object::SHIP) {

			const auto c = static_cast<Ship*>(*s);
			m_contacts.push_back(std::make_pair(c, c->ComputeOrbit()));
		}
	}
}

void SystemView::DrawShips(const double t, const vector3d &offset) {
	m_shipLabels->Clear();
	for(auto s = m_contacts.begin(); s != m_contacts.end(); s++) {
		const vector3d pos = offset + (*s).second.OrbitalPosAtTime(t) * double(m_zoom);
		const bool isNavTarget = Pi::player->GetNavTarget() == (*s).first;
		PutSelectionBox(pos, isNavTarget ? Color::GREEN : Color::BLUE);
		LabelShip((*s).first, pos);
		if(m_shipDrawing == ORBITS)
			PutOrbit(&(*s).second, offset, isNavTarget ? Color::GREEN : Color::BLUE, 0);
	}
}
