// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditor.h"

#include "EnumStrings.h"
#include "FileSystem.h"
#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "galaxy/AtmosphereParameters.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/Economy.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/NameGenerator.h"
#include "galaxy/Polit.h"
#include "galaxy/StarSystemGenerator.h"
#include "editor/UndoSystem.h"
#include "editor/UndoStepType.h"
#include "lua/Lua.h"

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include <memory>

using namespace Editor;

namespace {
	static constexpr const char *OUTLINE_WND_ID = "Outline";
	static constexpr const char *PROPERTIES_WND_ID = "Properties";
	static constexpr const char *VIEWPORT_WND_ID = "Viewport";

	bool InvalidSystemNameChar(char c)
	{
		return !(
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9'));
	}
}

namespace ImGui {
	bool InputFixed(const char *str, fixed *val, double step = 0.01, double step_fast = 0.1, const char *format = "%.4f", ImGuiInputTextFlags flags = 0)
	{
		double val_d = val->ToDouble();
		bool changed = ImGui::InputDouble(str, &val_d, step, step_fast, format, flags | ImGuiInputTextFlags_EnterReturnsTrue);
		if (changed)
			*val = fixed::FromDouble(val_d);

		return changed;
	}

	bool InputFixedAU(const char *str, fixed *val, double step = 0.01, double step_fast = 0.1)
	{
		return InputFixed(str, val, step, step_fast, "%.4f AU");
	}

	bool InputFixedDegrees(const char *str, fixed *val, double step = 1.0, double step_fast = 10.0, const char *format = "%.3f°", ImGuiInputTextFlags flags = 0)
	{
		double val_d = RAD2DEG(val->ToDouble());
		bool changed = ImGui::InputDouble(str, &val_d, step, step_fast, format, flags | ImGuiInputTextFlags_EnterReturnsTrue);
		if (changed)
			*val = fixed::FromDouble(DEG2RAD(val_d));

		return changed;
	}
};

class SystemBody::EditorAPI {
public:
	// Return a list of star types in the system; expects to be passed the root body
	static std::string GetStarTypes(SystemBody *body)
	{
		std::string types = "";

		if (body->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
			types = types + "'" + EnumStrings::GetString("BodyType", body->GetType()) + "', ";
		}

		for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
			types = types + GetStarTypes(body->m_children[ii]);
		}

		return types;
	}

	// NOTE: duplicated from StarSystem.cpp
	static std::string ExportToLua(FILE *f, SystemBody *body)
	{
		const int multiplier = 10000;

		// strip characters that will not work in Lua
		std::string code_name = body->GetName();
		std::transform(code_name.begin(), code_name.end(), code_name.begin(), ::tolower);
		code_name.erase(remove_if(code_name.begin(), code_name.end(), InvalidSystemNameChar), code_name.end());

		// Ensure we prepend a character to numbers to avoid generating an invalid identifier
		if (isdigit(code_name.front()))
			code_name = "body_" + code_name;

		// find the body type index so we can lookup the name
		const char *pBodyTypeName = EnumStrings::GetString("BodyType", body->GetType());

		if (body->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
			fprintf(f,
				"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
				"\t:latitude(math.deg2rad(%.1f))\n"
				"\t:longitude(math.deg2rad(%.1f))\n",

				code_name.c_str(),
				body->GetName().c_str(), pBodyTypeName,
				body->m_inclination.ToDouble() * 180 / M_PI,
				body->m_orbitalOffset.ToDouble() * 180 / M_PI);
		} else {
			fprintf(f,
				"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
				"\t:radius(f(%d,%d))\n"
				"\t:mass(f(%d,%d))\n",
				code_name.c_str(),
				body->GetName().c_str(), pBodyTypeName,
				int(round(body->GetRadiusAsFixed().ToDouble() * multiplier)), multiplier,
				int(round(body->GetMassAsFixed().ToDouble() * multiplier)), multiplier);

			if (body->GetAspectRatio() != 1.0) {
				fprintf(f,
					"\t:equatorial_to_polar_radius(f(%d,%d))\n",
					int(round(body->GetAspectRatio() * multiplier)), multiplier);
			}

			if (body->GetType() != SystemBody::TYPE_GRAVPOINT) {
				fprintf(f,
					"\t:seed(%u)\n"
					"\t:temp(%d)\n"
					"\t:semi_major_axis(f(%d,%d))\n"
					"\t:eccentricity(f(%d,%d))\n"
					"\t:rotation_period(f(%d,%d))\n"
					"\t:axial_tilt(fixed.deg2rad(f(%d,%d)))\n"
					"\t:rotational_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
					"\t:orbital_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
					"\t:orbital_offset(fixed.deg2rad(f(%d,%d)))\n",
					body->GetSeed(), body->GetAverageTemp(),
					int(round(body->GetOrbit().GetSemiMajorAxis() / AU * multiplier)), multiplier,
					int(round(body->GetOrbit().GetEccentricity() * multiplier)), multiplier,
					int(round(body->m_rotationPeriod.ToDouble() * multiplier)), multiplier,
					int(round(RAD2DEG(body->GetAxialTilt()) * multiplier)), multiplier,
					int(round(body->m_rotationalPhaseAtStart.ToDouble() * multiplier * 180 / M_PI)), multiplier,
					int(round(body->m_orbitalPhaseAtStart.ToDouble() * multiplier * 180 / M_PI)), multiplier,
					int(round(body->m_orbitalOffset.ToDouble() * multiplier * 180 / M_PI)), multiplier);

				if (body->GetInclinationAsFixed() != fixed(0, 0)) {
					fprintf(f,
						"\t:inclination(math.deg2rad(%f))\n",
						RAD2DEG(body->GetInclinationAsFixed().ToDouble()));
				}

			}

			if (body->GetType() == SystemBody::TYPE_PLANET_TERRESTRIAL)
				fprintf(f,
					"\t:metallicity(f(%d,%d))\n"
					"\t:volcanicity(f(%d,%d))\n"
					"\t:atmos_density(f(%d,%d))\n"
					"\t:atmos_oxidizing(f(%d,%d))\n"
					"\t:ocean_cover(f(%d,%d))\n"
					"\t:ice_cover(f(%d,%d))\n"
					"\t:life(f(%d,%d))\n",
					int(round(body->GetMetallicity() * multiplier)), multiplier,
					int(round(body->GetVolcanicity() * multiplier)), multiplier,
					int(round(body->GetVolatileGas() * multiplier)), multiplier,
					int(round(body->GetAtmosOxidizing() * multiplier)), multiplier,
					int(round(body->GetVolatileLiquid() * multiplier)), multiplier,
					int(round(body->GetVolatileIces() * multiplier)), multiplier,
					int(round(body->GetLife() * multiplier)), multiplier);
		}

		fprintf(f, "\n");

		std::string code_list = code_name;
		if (body->m_children.size() > 0) {
			code_list = code_list + ",\n\t{\n";
			for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
				code_list = code_list + "\t" + ExportToLua(f, body->m_children[ii]) + ",\n";
			}
			code_list = code_list + "\t}";
		}

		return code_list;
	}

	static void UpdateBodyOrbit(SystemBody *body)
	{
		body->m_orbMin = body->m_semiMajorAxis - body->m_eccentricity * body->m_semiMajorAxis;
		body->m_orbMax = 2 * body->m_semiMajorAxis - body->m_orbMin;

		if (body->m_parent)
			UpdateOrbitAroundParent(body, body->m_parent);
	}

	static void UpdateOrbitAroundParent(SystemBody *body, SystemBody *parent)
	{
		if (parent->GetType() == SystemBody::TYPE_GRAVPOINT) // generalize Kepler's law to multiple stars
			body->m_orbit.SetShapeAroundBarycentre(body->m_semiMajorAxis.ToDouble() * AU, parent->GetMass(), body->GetMass(), body->m_eccentricity.ToDouble());
		else
			body->m_orbit.SetShapeAroundPrimary(body->m_semiMajorAxis.ToDouble() * AU, parent->GetMass(), body->m_eccentricity.ToDouble());

		body->m_orbit.SetPhase(body->m_orbitalPhaseAtStart.ToDouble());

		double latitude = body->m_inclination.ToDouble();
		double longitude = body->m_orbitalOffset.ToDouble();
		body->m_orbit.SetPlane(matrix3x3d::RotateY(longitude) * matrix3x3d::RotateX(-0.5 * M_PI + latitude));
	}

	static void EditOrbitalParameters(SystemBody *body, UndoSystem *undo)
	{
		ImGui::SeparatorText("Orbital Parameters");

		bool orbitChanged = false;

		orbitChanged |= ImGui::InputFixedAU("Semi-Major Axis", &body->m_semiMajorAxis);
		if (Draw::UndoHelper("Edit Semi-Major Axis", undo))
			AddUndoSingleValueClosure(undo, &body->m_semiMajorAxis, [=](){ UpdateBodyOrbit(body); });

		orbitChanged |= ImGui::InputFixed("Eccentricity", &body->m_eccentricity);
		if (Draw::UndoHelper("Edit Eccentricity", undo))
			AddUndoSingleValueClosure(undo, &body->m_eccentricity, [=](){ UpdateBodyOrbit(body); });

		ImGui::BeginDisabled();
		ImGui::InputFixed("Periapsis", &body->m_orbMin, 0.0, 0.0, "%0.6f AU");
		ImGui::InputFixed("Apoapsis", &body->m_orbMax, 0.0, 0.0, "%0.6f AU");
		ImGui::EndDisabled();

		orbitChanged |= ImGui::InputFixedDegrees("Axial Tilt", &body->m_axialTilt);
		if (Draw::UndoHelper("Edit Axial Tilt", undo))
			AddUndoSingleValue(undo, &body->m_axialTilt);

		orbitChanged |= ImGui::InputFixedDegrees("Inclination", &body->m_inclination);
		if (Draw::UndoHelper("Edit Inclination", undo))
			AddUndoSingleValue(undo, &body->m_inclination);

		orbitChanged |= ImGui::InputFixedDegrees("Orbital Offset", &body->m_orbitalOffset);
		if (Draw::UndoHelper("Edit Orbital Offset", undo))
			AddUndoSingleValue(undo, &body->m_orbitalOffset);

		orbitChanged |= ImGui::InputFixedDegrees("Orbital Phase at Start", &body->m_orbitalPhaseAtStart);
		if (Draw::UndoHelper("Edit Orbital Phase at Start", undo))
			AddUndoSingleValue(undo, &body->m_orbitalPhaseAtStart);

		orbitChanged |= ImGui::InputFixedDegrees("Rotation at Start", &body->m_rotationalPhaseAtStart);
		if (Draw::UndoHelper("Edit Rotational Phase at Start", undo))
			AddUndoSingleValue(undo, &body->m_rotationalPhaseAtStart);

		orbitChanged |= ImGui::InputFixed("Rotation Period (Days)", &body->m_rotationPeriod, 1.0, 10.0);
		if (Draw::UndoHelper("Edit Rotation Period", undo))
			AddUndoSingleValue(undo, &body->m_rotationPeriod);

		if (orbitChanged)
			UpdateBodyOrbit(body);
	}

	static void EditEconomicProperties(SystemBody *body, UndoSystem *undo)
	{
		ImGui::SeparatorText("Economic Parameters");

		ImGui::InputFixed("Population", &body->m_population);
		if (Draw::UndoHelper("Edit Population", undo))
			AddUndoSingleValue(undo, &body->m_population);

		ImGui::InputFixed("Agricultural Activity", &body->m_agricultural);
		if (Draw::UndoHelper("Edit Agricultural Activity", undo))
			AddUndoSingleValue(undo, &body->m_agricultural);
	}

	static void EditStarportProperties(SystemBody *body, UndoSystem *undo)
	{
		if (body->GetType() == TYPE_STARPORT_SURFACE) {
			ImGui::SeparatorText("Surface Parameters");

			ImGui::InputFixedDegrees("Latitude", &body->m_inclination);
			if (Draw::UndoHelper("Edit Latitude", undo))
				AddUndoSingleValue(undo, &body->m_inclination);

			ImGui::InputFixedDegrees("Longitude", &body->m_orbitalOffset);
			if (Draw::UndoHelper("Edit Longitude", undo))
				AddUndoSingleValue(undo, &body->m_orbitalOffset);
		} else {
			EditOrbitalParameters(body, undo);
		}

		EditEconomicProperties(body, undo);
	}

	static void EditProperties(SystemBody *body, UndoSystem *undo)
	{
		bool isStar = body->GetSuperType() <= SUPERTYPE_STAR;

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5);

		ImGui::InputText("Name", &body->m_name);
		if (Draw::UndoHelper("Edit Name", undo))
			AddUndoSingleValue(undo, &body->m_name);

		Draw::EditEnum("Edit Body Type", "Body Type", "BodyType", reinterpret_cast<int *>(&body->m_type), BodyType::TYPE_MAX, undo);

		ImGui::InputInt("Seed", reinterpret_cast<int *>(&body->m_seed));
		if (Draw::UndoHelper("Edit Seed", undo))
			AddUndoSingleValue(undo, &body->m_seed);

		if (body->GetSuperType() < SUPERTYPE_STARPORT) {
			ImGui::InputFixed(isStar ? "Radius (sol)" : "Radius (earth)", &body->m_radius);
			if (Draw::UndoHelper("Edit Radius", undo))
				AddUndoSingleValue(undo, &body->m_radius);

			ImGui::InputFixed("Aspect Ratio", &body->m_aspectRatio);
			if (Draw::UndoHelper("Edit Aspect Ratio", undo))
				AddUndoSingleValue(undo, &body->m_aspectRatio);

			ImGui::InputFixed(isStar ? "Mass (sol)" : "Mass (earth)", &body->m_mass);
			if (Draw::UndoHelper("Edit Mass", undo))
				AddUndoSingleValue(undo, &body->m_mass);

			ImGui::InputInt("Temperature (K)", &body->m_averageTemp, 1, 10);
			if (Draw::UndoHelper("Edit Temperature", undo))
				AddUndoSingleValue(undo, &body->m_averageTemp);

		} else {
			EditStarportProperties(body, undo);

			ImGui::PopItemWidth();
			return;
		}

		// TODO: orbital parameters not needed for root body

		EditOrbitalParameters(body, undo);

		if (isStar) {
			ImGui::PopItemWidth();
			return;
		}

		ImGui::SeparatorText("Surface Parameters");

		ImGui::InputFixed("Metallicity", &body->m_metallicity);
		if (Draw::UndoHelper("Edit Metallicity", undo))
			AddUndoSingleValue(undo, &body->m_metallicity);

		ImGui::InputFixed("Volcanicity", &body->m_volcanicity);
		if (Draw::UndoHelper("Edit Volcanicity", undo))
			AddUndoSingleValue(undo, &body->m_volcanicity);

		ImGui::InputFixed("Atmosphere Density", &body->m_volatileGas);
		if (Draw::UndoHelper("Edit Atmosphere Density", undo))
			AddUndoSingleValue(undo, &body->m_volatileGas);

		ImGui::InputFixed("Atmosphere Oxidizing", &body->m_atmosOxidizing);
		if (Draw::UndoHelper("Edit Atmosphere Oxidizing", undo))
			AddUndoSingleValue(undo, &body->m_atmosOxidizing);

		ImGui::InputFixed("Ocean Coverage", &body->m_volatileLiquid);
		if (Draw::UndoHelper("Edit Ocean Coverage", undo))
			AddUndoSingleValue(undo, &body->m_volatileLiquid);

		ImGui::InputFixed("Ice Coverage", &body->m_volatileIces);
		if (Draw::UndoHelper("Edit Ice Coverage", undo))
			AddUndoSingleValue(undo, &body->m_volatileIces);

		// TODO: unused by other code
		// ImGui::InputFixed("Human Activity", &body->m_humanActivity);
		// if (Draw::UndoHelper("Edit Human Activity", undo))
		// 	AddUndoSingleValue(undo, &body->m_humanActivity);

		ImGui::InputFixed("Life", &body->m_life);
		if (Draw::UndoHelper("Edit Life", undo))
			AddUndoSingleValue(undo, &body->m_life);

		EditEconomicProperties(body, undo);

		ImGui::PopItemWidth();
	}
};

class StarSystem::EditorAPI {
public:
	static void ExportToLua(FILE *f, StarSystem *system, Galaxy *galaxy)
	{
		fprintf(f, "-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details\n");
		fprintf(f, "-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt\n\n");

		SystemBody *rootBody = system->GetRootBody().Get();

		std::string stars_in_system = SystemBody::EditorAPI::GetStarTypes(rootBody);

		const char *govType = EnumStrings::GetString("PolitGovType", system->GetSysPolit().govType);

		fprintf(f, "local system = CustomSystem:new('%s', { %s })\n\t:govtype('%s')\n\t:short_desc('%s')\n\t:long_desc([[%s]])\n\n",
			system->GetName().c_str(), stars_in_system.c_str(), govType, system->GetShortDescription().c_str(), system->GetLongDescription().c_str());

		fprintf(f, "system:bodies(%s)\n\n", SystemBody::EditorAPI::ExportToLua(f, rootBody).c_str());

		SystemPath pa = system->GetPath();
		RefCountedPtr<const Sector> sec = galaxy->GetSector(pa);

		fprintf(f, "system:add_to_sector(%d,%d,%d,v(%.4f,%.4f,%.4f))\n",
			pa.sectorX, pa.sectorY, pa.sectorZ,
			sec->m_systems[pa.systemIndex].GetPosition().x / Sector::SIZE,
			sec->m_systems[pa.systemIndex].GetPosition().y / Sector::SIZE,
			sec->m_systems[pa.systemIndex].GetPosition().z / Sector::SIZE);
	}

	static void EditName(StarSystem *system, Random &rng, UndoSystem *undo)
	{
		float buttonSize = ImGui::GetFrameHeight();
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonSize - ImGui::GetStyle().ItemSpacing.x);

		ImGui::InputText("##Name", &system->m_name);
		if (Draw::UndoHelper("Edit System Name", undo))
			AddUndoSingleValue(undo, &system->m_name);

		ImGui::SameLine();
		if (ImGui::Button("R", ImVec2(buttonSize, buttonSize))) {
			system->m_name.clear();
			NameGenerator::GetSystemName(*&system->m_name, rng);
		}
		if (Draw::UndoHelper("Edit System Name", undo))
			AddUndoSingleValue(undo, &system->m_name);

		ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::TextUnformatted("Name");
	}

	static void EditProperties(StarSystem *system, UndoSystem *undo)
	{
		// TODO: other names

		ImGui::InputText("Short Description", &system->m_shortDesc);
		if (Draw::UndoHelper("Edit System Short Description", undo))
			AddUndoSingleValue(undo, &system->m_shortDesc);

		ImGui::InputTextMultiline("Long Description", &system->m_longDesc, ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 5));
		if (Draw::UndoHelper("Edit System Long Description", undo))
			AddUndoSingleValue(undo, &system->m_longDesc);

		ImGui::SeparatorText("Generation Parameters");

		ImGui::InputInt("Seed", reinterpret_cast<int *>(&system->m_seed));
		if (Draw::UndoHelper("Edit Seed", undo))
			AddUndoSingleValue(undo, &system->m_seed);

		bool explored = system->m_explored == ExplorationState::eEXPLORED_AT_START;
		ImGui::Checkbox("Explored", &explored);
		if (Draw::UndoHelper("Edit System Explored", undo))
			AddUndoSingleValue(undo, &system->m_explored, explored ? eEXPLORED_AT_START : eUNEXPLORED);

		ImGui::SeparatorText("Economic Parameters");

		// TODO: faction

		Draw::EditEnum("Edit System Government", "Government", "PolitGovType",
			reinterpret_cast<int *>(&system->m_polit.govType), Polit::GovType::GOV_MAX - 1, undo);

		ImGui::InputFixed("Lawlessness", &system->m_polit.lawlessness);
		if (Draw::UndoHelper("Edit System Lawlessness", undo))
			AddUndoSingleValue(undo, &system->m_polit.lawlessness);
	}
};

SystemEditor::SystemEditor(EditorApp *app) :
	m_app(app),
	m_undo(new UndoSystem()),
	m_selectedBody(nullptr)
{
	GalacticEconomy::Init();

	m_galaxy = GalaxyGenerator::Create();
	m_systemLoader.reset(new CustomSystemsDatabase(m_galaxy.Get(), "systems"));

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

SystemEditor::~SystemEditor()
{
}

bool SystemEditor::LoadSystem(const std::string &filepath)
{
	const CustomSystem *csys = m_systemLoader->LoadSystem(filepath);
	if (!csys)
		return false;

	SystemPath path = {csys->sectorX, csys->sectorY, csys->sectorZ, csys->systemIndex};
	Uint32 _init[6] = { csys->systemIndex, Uint32(csys->sectorX), Uint32(csys->sectorY), Uint32(csys->sectorZ), UNIVERSE_SEED, Uint32(csys->seed) };
	Random rng(_init, 6);

	RefCountedPtr<StarSystem::GeneratorAPI> system(new StarSystem::GeneratorAPI(path, m_galaxy, nullptr, rng));
	auto customStage = std::make_unique<StarSystemCustomGenerator>();

	if (!customStage->ApplyToSystem(rng, system, csys)) {
		Log::Error("System is fully random, cannot load from file");
		return false;
	}

	// FIXME: need to run StarSystemPopulateGenerator here to finish filling out system
	// Setting up faction affinity etc. requires running full gamut of generator stages

	// auto populateStage = std::make_unique<PopulateStarSystemGenerator>();
	// GalaxyGenerator::StarSystemConfig config;
	// config.isCustomOnly = true;

	// populateStage->Apply(rng, m_galaxy, system, &config);

	if (!system->GetRootBody()) {
		Log::Error("Custom system doesn't have a root body");
		return false;
	}

	m_system = system;
	m_filepath = filepath;

	return true;
}

void SystemEditor::WriteSystem(const std::string &filepath)
{
	Log::Info("Writing to path: {}/{}", FileSystem::GetDataDir(), filepath);
	// FIXME: need better file-saving interface for the user
	FILE *f = FileSystem::FileSourceFS(FileSystem::GetDataDir()).OpenWriteStream(filepath, FileSystem::FileSourceFS::WRITE_TEXT);

	if (!f)
		return;

	StarSystem::EditorAPI::ExportToLua(f, m_system.Get(), m_galaxy.Get());

	fclose(f);
}

void SystemEditor::Start()
{
}

void SystemEditor::End()
{
}

// ─── Update Loop ─────────────────────────────────────────────────────────────

void SystemEditor::HandleInput()
{

}

void SystemEditor::Update(float deltaTime)
{
	ImGuiID editorID = ImGui::GetID("System Editor");
	if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, editorID, ImGuiInputFlags_RouteGlobal)) {
		GetUndo()->Redo();
	} else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, editorID, ImGuiInputFlags_RouteGlobal)) {
		GetUndo()->Undo();
	}

	DrawInterface();

	if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
		WriteSystem(m_filepath);
	}
}

// ─── Interface Rendering ─────────────────────────────────────────────────────

void SystemEditor::SetupLayout(ImGuiID dockspaceID)
{
	ImGuiID nodeID = ImGui::DockBuilderAddNode(dockspaceID);

	ImGui::DockBuilderSetNodePos(nodeID, ImGui::GetWindowPos());
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetWindowSize());

	ImGuiID leftSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Left, 0.25, nullptr, &nodeID);
	ImGuiID rightSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Right, 0.25 / (1.0 - 0.25), nullptr, &nodeID);
	// ImGuiID bottom = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Down, 0.2, nullptr, &nodeID);

	ImGui::DockBuilderDockWindow(OUTLINE_WND_ID, leftSide);
	ImGui::DockBuilderDockWindow(PROPERTIES_WND_ID, rightSide);
	ImGui::DockBuilderDockWindow(VIEWPORT_WND_ID, nodeID);

	ImGui::DockBuilderFinish(dockspaceID);
}

void SystemEditor::DrawInterface()
{
	Draw::ShowUndoDebugWindow(GetUndo());

	static bool isFirstRun = true;

	Draw::BeginHostWindow("HostWindow", nullptr, ImGuiWindowFlags_NoSavedSettings);

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");

	if (isFirstRun)
		SetupLayout(dockspaceID);

	ImGui::DockSpace(dockspaceID);

	if (ImGui::Begin(OUTLINE_WND_ID)) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 14));
		DrawOutliner();
		ImGui::PopFont();
	}
	ImGui::End();

	if (ImGui::Begin(PROPERTIES_WND_ID)) {
		if (m_selectedBody)
			DrawBodyProperties();
		else
			DrawSystemProperties();
	}
	ImGui::End();

	ImGui::End();

	if (isFirstRun)
		isFirstRun = false;
}

void SystemEditor::DrawOutliner()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	std::string label = fmt::format("System: {}", m_system->GetName());
	if (ImGui::Selectable(label.c_str(), !m_selectedBody)) {
		m_selectedBody = nullptr;
	}

	ImGui::PopFont();

	ImGui::Spacing();

	if (ImGui::BeginChild("OutlinerList")) {
		std::vector<std::pair<SystemBody *, size_t>> m_systemStack {
			{ m_system->GetRootBody().Get(), 0 }
		};

		if (!DrawBodyNode(m_system->GetRootBody().Get())) {
			ImGui::EndChild();
			return;
		}

		while (!m_systemStack.empty()) {
			auto &pair = m_systemStack.back();

			if (pair.second == pair.first->GetNumChildren()) {
				m_systemStack.pop_back();
				ImGui::TreePop();
				continue;
			}

			SystemBody *body = pair.first->GetChildren()[pair.second++];
			if (DrawBodyNode(body))
				m_systemStack.push_back({ body, 0 });
		}
	}
	ImGui::EndChild();
}

bool SystemEditor::DrawBodyNode(SystemBody *body)
{
	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanFullWidth;

	if (body->GetNumChildren() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	if (body == m_selectedBody)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(body->GetName().c_str(), flags);

	if (ImGui::IsItemActivated()) {
		m_selectedBody = body;
	}

	// TODO: custom rendering on body entry, e.g. icon / contents etc.

	return open && body->GetNumChildren();
}

void SystemEditor::DrawBodyProperties()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	ImGui::Text("Body: %s (%d)", m_selectedBody->GetName().c_str(), m_selectedBody->GetPath().bodyIndex);
	ImGui::PopFont();

	ImGui::Spacing();

	SystemBody::EditorAPI::EditProperties(m_selectedBody, GetUndo());
}

void SystemEditor::DrawSystemProperties()
{
	if (!m_system) {
		ImGui::Text("No loaded system");
		return;
	}

	SystemPath path = m_system->GetPath();

	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	ImGui::Text("%s (%d, %d, %d : %d)",
		m_system->GetName().c_str(),
		path.sectorX, path.sectorY, path.sectorZ, path.systemIndex);
	ImGui::PopFont();

	ImGui::Spacing();

	Random rng (Uint32(m_app->GetTime() * 4.0) ^ m_system->GetSeed());
	StarSystem::EditorAPI::EditName(m_system.Get(), rng, GetUndo());

	StarSystem::EditorAPI::EditProperties(m_system.Get(), GetUndo());
}
