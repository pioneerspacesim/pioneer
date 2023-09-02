// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyEditAPI.h"

#include "SystemEditorHelpers.h"

#include "core/Log.h"
#include "editor/UndoStepType.h"
#include "editor/EditorDraw.h"

#include "EnumStrings.h"
#include "galaxy/Sector.h"
#include "galaxy/Galaxy.h"
#include "galaxy/NameGenerator.h"
#include "galaxy/StarSystemGenerator.h"

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

using namespace Editor;

namespace {
	bool InvalidSystemNameChar(char c)
	{
		return !(
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9'));
	}

	static constexpr double SECONDS_TO_DAYS = 1.0 / (3600.0 * 24.0);
}

namespace Editor::Draw {

	// Essentially CollapsingHeader without the frame and with consistent ID regardless of edited body
	bool DerivedValues(std::string_view sectionLabel) {
		constexpr ImGuiID detailSeed = "Editor Details"_hash32;

		if (ImGui::GetCurrentWindow()->SkipItems)
			return false;

		ImGuiID treeId = ImGui::GetIDWithSeed(sectionLabel.data(), sectionLabel.data() + sectionLabel.size(), detailSeed);
		return ImGui::TreeNodeBehavior(treeId, ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog, "Derived Values");
	}

} // namespace Editor::Draw

void StarSystem::EditorAPI::ExportToLua(FILE *f, StarSystem *system, Galaxy *galaxy)
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

SystemBody *StarSystem::EditorAPI::NewBody(StarSystem *system)
{
	return system->NewBody();
}

SystemBody *StarSystem::EditorAPI::NewBodyAround(StarSystem *system, Random &rng, SystemBody *primary, size_t idx)
{
	StarSystemRandomGenerator gen = {};

	// Ensure consistent density distribution parameters across multiple runs
	const SystemPath &path = system->GetPath();
	Random shellRng { StarSystemRandomGenerator::BODY_SATELLITE_SALT, primary->GetSeed(), uint32_t(path.sectorX), uint32_t(path.sectorY), uint32_t(path.sectorZ), UNIVERSE_SEED };

	fixed discMin, discBound, discMax;
	fixed discDensity = gen.CalcBodySatelliteShellDensity(shellRng, primary, discMin, discMax);

	discBound = fixed(0);

	size_t numChildren = primary->GetNumChildren();

	// Set orbit slice parameters from surrounding bodies
	if (idx > 0)
		discMin = primary->m_children[idx - 1]->m_orbMax * fixed(105, 100);
	if (idx < numChildren)
		discBound = primary->m_children[idx]->m_orbMin;

	// Ensure we have enough discMax to generate a body, even if it means "cheating"

	if (discMin * fixed(12, 10) > discMax) {
		Log::Warning("Creating body outside of parent {} natural satellite radius {:.8f}, generation may not be correct.", primary->GetName(), discMax.ToDouble());
		discMax = numChildren > 0 ? primary->m_children[numChildren - 1]->m_orbMax : discMin;
		discMax *= fixed(12, 10);
	}

	if (discMin > discMax || (discBound != 0 && discMin > discBound))
		return nullptr;

	SystemBody *body = gen.MakeBodyInOrbitSlice(rng, static_cast<StarSystem::GeneratorAPI *>(system), primary, discMin, discBound, discMax, discDensity);
	if (!body)
		return nullptr;

	gen.PickPlanetType(body, rng);

	return body;
}

void StarSystem::EditorAPI::AddBody(StarSystem *system, SystemBody *body, size_t idx)
{
	if (idx == size_t(-1))
		idx = system->m_bodies.size();

	auto iter = system->m_bodies.begin() + idx;
	system->m_bodies.emplace(iter, body);
}

void StarSystem::EditorAPI::RemoveBody(StarSystem *system, SystemBody *body)
{
	auto iter = std::find(system->m_bodies.begin(), system->m_bodies.end(), body);
	if (iter != system->m_bodies.end())
		system->m_bodies.erase(iter);
}

void StarSystem::EditorAPI::ReorderBodyIndex(StarSystem *system)
{
	size_t index = 0;
	system->GetRootBody()->m_path.bodyIndex = index++;

	std::vector<std::pair<SystemBody *, size_t>> orderStack {
		{ system->GetRootBody().Get(), 0 }
	};

	while (!orderStack.empty()) {
		auto &pair = orderStack.back();

		if (pair.second >= pair.first->GetNumChildren()) {
			orderStack.pop_back();
			continue;
		}

		SystemBody *body = pair.first->GetChildren()[pair.second++];
		orderStack.push_back({ body, 0 });

		body->m_path.bodyIndex = index ++;
	}

	std::sort(system->m_bodies.begin(), system->m_bodies.end(), [](auto a, auto b) {
		return a->m_path.bodyIndex < b->m_path.bodyIndex;
	});
}

void StarSystem::EditorAPI::EditName(StarSystem *system, Random &rng, UndoSystem *undo)
{
	ImGui::BeginGroup();
	if (Draw::RandomButton()) {
		system->m_name.clear();
		NameGenerator::GetSystemName(*&system->m_name, rng);
	}

	ImGui::InputText("Name", &system->m_name);
	ImGui::EndGroup();

	if (Draw::UndoHelper("Edit System Name", undo))
		AddUndoSingleValue(undo, &system->m_name);
}

void StarSystem::EditorAPI::EditProperties(StarSystem *system, UndoSystem *undo)
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

	Draw::InputFixedSlider("Lawlessness", &system->m_polit.lawlessness);
	if (Draw::UndoHelper("Edit System Lawlessness", undo))
		AddUndoSingleValue(undo, &system->m_polit.lawlessness);
}

// ─── SystemBody::EditorAPI ───────────────────────────────────────────────────

// Return a list of star types in the system; expects to be passed the root body
std::string SystemBody::EditorAPI::GetStarTypes(SystemBody *body)
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
std::string SystemBody::EditorAPI::ExportToLua(FILE *f, SystemBody *body)
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
			"\t:seed(%d)"
			"\t:latitude(math.deg2rad(%.1f))\n"
			"\t:longitude(math.deg2rad(%.1f))\n",

			code_name.c_str(),
			body->GetName().c_str(), pBodyTypeName,
			body->m_seed,
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


void SystemBody::EditorAPI::AddChild(SystemBody *parent, SystemBody *child, size_t idx)
{
	if (idx == size_t(-1))
		idx = parent->m_children.size();

	auto iter = parent->m_children.begin() + idx;
	parent->m_children.emplace(iter, child);

	child->m_parent = parent;
}

SystemBody *SystemBody::EditorAPI::RemoveChild(SystemBody *parent, size_t idx)
{
	if (idx == size_t(-1))
		idx = parent->m_children.size() - 1;

	SystemBody *outBody = parent->m_children[idx];
	parent->m_children.erase(parent->m_children.begin() + idx);

	outBody->m_parent = nullptr;

	return outBody;
}

size_t SystemBody::EditorAPI::GetIndexInParent(SystemBody *body)
{
	SystemBody *parent = body->GetParent();
	if (!parent)
		return size_t(-1);

	auto iter = std::find(parent->GetChildren().begin(), parent->GetChildren().end(), body);
	if (iter == parent->GetChildren().end())
		return size_t(-1);

	return std::distance(parent->GetChildren().begin(), iter);
}

void SystemBody::EditorAPI::EditOrbitalParameters(SystemBody *body, UndoSystem *undo)
{
	ImGui::SeparatorText("Orbital Parameters");

	bool orbitChanged = false;
	auto updateBodyOrbit = [=](){ body->SetOrbitFromParameters(); };

	orbitChanged |= Draw::InputFixedDistance("Semi-Major Axis", &body->m_semiMajorAxis);
	if (Draw::UndoHelper("Edit Semi-Major Axis", undo))
		AddUndoSingleValueClosure(undo, &body->m_semiMajorAxis, updateBodyOrbit);

	orbitChanged |= Draw::InputFixedSlider("Eccentricity", &body->m_eccentricity);
	if (Draw::UndoHelper("Edit Eccentricity", undo))
		AddUndoSingleValueClosure(undo, &body->m_eccentricity, updateBodyOrbit);

	orbitChanged |= Draw::InputFixedDegrees("Inclination", &body->m_inclination, 0.0, 180.0);
	if (Draw::UndoHelper("Edit Inclination", undo))
		AddUndoSingleValueClosure(undo, &body->m_inclination, updateBodyOrbit);

	orbitChanged |= Draw::InputFixedDegrees("Orbital Offset", &body->m_orbitalOffset);
	if (Draw::UndoHelper("Edit Orbital Offset", undo))
		AddUndoSingleValueClosure(undo, &body->m_orbitalOffset, updateBodyOrbit);
	Draw::HelpMarker("Longitude of Ascending Node");

	orbitChanged |= Draw::InputFixedDegrees("Arg. of Periapsis", &body->m_argOfPeriapsis);
	if (Draw::UndoHelper("Edit Argument of Periapsis", undo))
		AddUndoSingleValueClosure(undo, &body->m_argOfPeriapsis, updateBodyOrbit);
	Draw::HelpMarker("Argument of Periapsis\nRelative to Longitude of Ascending Node");

	orbitChanged |= Draw::InputFixedDegrees("Orbital Phase", &body->m_orbitalPhaseAtStart);
	if (Draw::UndoHelper("Edit Orbital Phase", undo))
		AddUndoSingleValueClosure(undo, &body->m_orbitalPhaseAtStart, updateBodyOrbit);
	Draw::HelpMarker("True Anomaly at Epoch\nRelative to Argument of Periapsis");


	if (Draw::DerivedValues("Orbital Parameters")) {
		ImGui::BeginDisabled();

		ImGui::InputFixed("Periapsis", &body->m_orbMin, 0.0, 0.0, "%0.6f AU");
		ImGui::InputFixed("Apoapsis", &body->m_orbMax, 0.0, 0.0, "%0.6f AU");

		double orbit_period = body->GetOrbit().Period() * SECONDS_TO_DAYS;
		ImGui::InputDouble("Orbital Period", &orbit_period, 0.0, 0.0, "%.2f days");

		if (body->GetParent()) {
			// calculate the time offset from periapsis at epoch
			double orbit_time_at_start = (body->GetOrbit().GetOrbitalPhaseAtStart() / (2.0 * M_PI)) * body->GetOrbit().Period();

			double orbit_vel_ap = body->GetOrbit().OrbitalVelocityAtTime(
				body->GetParent()->GetMass(),
				body->GetOrbit().Period() * 0.5 - orbit_time_at_start).Length() / 1000.0;

			double orbit_vel_pe = body->GetOrbit().OrbitalVelocityAtTime(
				body->GetParent()->GetMass(),
				-orbit_time_at_start).Length() / 1000.0;

			ImGui::InputDouble("Orbital Velocity (AP)", &orbit_vel_ap, 0.0, 0.0, "%.2f km/s");
			ImGui::InputDouble("Orbital Velocity (PE)", &orbit_vel_pe, 0.0, 0.0, "%.2f km/s");
		}

		ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Rotation Parameters");

	orbitChanged |= Draw::InputFixedDegrees("Axial Tilt", &body->m_axialTilt);
	if (Draw::UndoHelper("Edit Axial Tilt", undo))
		AddUndoSingleValueClosure(undo, &body->m_axialTilt, updateBodyOrbit);

	orbitChanged |= Draw::InputFixedDegrees("Rotation at Start", &body->m_rotationalPhaseAtStart);
	if (Draw::UndoHelper("Edit Rotational Phase at Start", undo))
		AddUndoSingleValueClosure(undo, &body->m_rotationalPhaseAtStart, updateBodyOrbit);

	orbitChanged |= ImGui::InputFixed("Rotation Period", &body->m_rotationPeriod, 1.0, 10.0, "%.3f days");
	if (Draw::UndoHelper("Edit Rotation Period", undo))
		AddUndoSingleValueClosure(undo, &body->m_rotationPeriod, updateBodyOrbit);

	if (orbitChanged)
		body->SetOrbitFromParameters();

}

void SystemBody::EditorAPI::EditEconomicProperties(SystemBody *body, UndoSystem *undo)
{
	ImGui::SeparatorText("Economic Parameters");

	ImGui::InputFixed("Population", &body->m_population);
	if (Draw::UndoHelper("Edit Population", undo))
		AddUndoSingleValue(undo, &body->m_population);

	ImGui::InputFixed("Agricultural Activity", &body->m_agricultural);
	if (Draw::UndoHelper("Edit Agricultural Activity", undo))
		AddUndoSingleValue(undo, &body->m_agricultural);
}

void SystemBody::EditorAPI::EditStarportProperties(SystemBody *body, UndoSystem *undo)
{
	if (body->GetType() == TYPE_STARPORT_SURFACE) {
		ImGui::SeparatorText("Surface Parameters");

		Draw::InputFixedDegrees("Latitude", &body->m_inclination);
		if (Draw::UndoHelper("Edit Latitude", undo))
			AddUndoSingleValue(undo, &body->m_inclination);

		Draw::InputFixedDegrees("Longitude", &body->m_orbitalOffset);
		if (Draw::UndoHelper("Edit Longitude", undo))
			AddUndoSingleValue(undo, &body->m_orbitalOffset);
	} else {
		EditOrbitalParameters(body, undo);
	}

	EditEconomicProperties(body, undo);
}

void SystemBody::EditorAPI::EditProperties(SystemBody *body, Random &rng, UndoSystem *undo)
{
	bool isStar = body->GetSuperType() <= SUPERTYPE_STAR;

	bool bodyChanged = false;
	auto updateBodyDerived = [=]() {
		body->SetAtmFromParameters();
	};

	ImGui::InputText("Name", &body->m_name);
	if (Draw::UndoHelper("Edit Name", undo))
		AddUndoSingleValue(undo, &body->m_name);

	Draw::EditEnum("Edit Body Type", "Body Type", "BodyType", reinterpret_cast<int *>(&body->m_type), BodyType::TYPE_MAX, undo);

	ImGui::InputInt("Seed", reinterpret_cast<int *>(&body->m_seed));
	if (Draw::UndoHelper("Edit Seed", undo))
		AddUndoSingleValue(undo, &body->m_seed);

	if (body->GetSuperType() < SUPERTYPE_STARPORT) {

		if (!isStar && ImGui::Button(EICON_RANDOM " Body Stats")) {
			GenerateDerivedStats(body, rng, undo);
			bodyChanged = true;
		}

		ImGui::SetItemTooltip("Generate body type, radius, temperature, and surface parameters using the same method as procedural system generation.");

		ImGui::SeparatorText("Body Parameters");

		bodyChanged |= Draw::InputFixedMass("Mass", &body->m_mass, isStar);
		if (Draw::UndoHelper("Edit Mass", undo))
			AddUndoSingleValueClosure(undo, &body->m_mass, updateBodyDerived);

		bodyChanged |= Draw::InputFixedRadius("Radius",  &body->m_radius, isStar);
		if (Draw::UndoHelper("Edit Radius", undo))
			AddUndoSingleValueClosure(undo, &body->m_radius, updateBodyDerived);

		ImGui::BeginDisabled();

		double surfaceGrav = body->CalcSurfaceGravity();
		ImGui::InputDouble("Surface Gravity", &surfaceGrav, 0, 0, "%.4fg");

		ImGui::EndDisabled();

		if (body->GetSuperType() <= SUPERTYPE_GAS_GIANT && body->GetType() != TYPE_PLANET_ASTEROID) {

			Draw::InputFixedSlider("Aspect Ratio", &body->m_aspectRatio, 0.0, 2.0);
			if (Draw::UndoHelper("Edit Aspect Ratio", undo))
				AddUndoSingleValue(undo, &body->m_aspectRatio);

			Draw::HelpMarker("Ratio of body equatorial radius to polar radius, or \"bulge\" around axis of spin.");

		}

		bodyChanged |= ImGui::InputInt("Temperature (K)", &body->m_averageTemp, 1, 10, "%d°K");
		if (Draw::UndoHelper("Edit Temperature", undo))
			AddUndoSingleValueClosure(undo, &body->m_averageTemp, updateBodyDerived);

		ImGui::Spacing();

		const bool hasDerived = (body->GetType() != TYPE_GRAVPOINT || body->HasChildren());

		if (hasDerived && Draw::DerivedValues("Body Parameters")) {
			ImGui::BeginDisabled();

			StarSystemRandomGenerator gen = {};

			// Ensure consistent density distribution parameters across multiple runs
			const SystemPath &path = body->GetStarSystem()->GetPath();
			Random shellRng { StarSystemRandomGenerator::BODY_SATELLITE_SALT, body->GetSeed(), uint32_t(path.sectorX), uint32_t(path.sectorY), uint32_t(path.sectorZ), UNIVERSE_SEED };

			fixed discMin, discBound, discMax;
			fixed discDensity = gen.CalcBodySatelliteShellDensity(shellRng, body, discMin, discMax);

			Draw::InputFixedDistance("Satellite Shell Min", &discMin);
			Draw::InputFixedDistance("Satellite Shell Max", &discMax);
			ImGui::InputFixed("Shell Density Dist", &discDensity, 0, 0, "%.6f");

			ImGui::EndDisabled();
		}

	} else {
		EditStarportProperties(body, undo);
		return;
	}

	if (body->GetParent()) {
		EditOrbitalParameters(body, undo);
	}

	if (isStar) {
		return;
	}

	ImGui::SeparatorText("Surface Parameters");

	Draw::InputFixedSlider("Metallicity", &body->m_metallicity);
	if (Draw::UndoHelper("Edit Metallicity", undo))
		AddUndoSingleValue(undo, &body->m_metallicity);

	Draw::InputFixedSlider("Volcanicity", &body->m_volcanicity);
	if (Draw::UndoHelper("Edit Volcanicity", undo))
		AddUndoSingleValue(undo, &body->m_volcanicity);

	bool gasGiant = body->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT;

	bodyChanged |= Draw::InputFixedSlider("Atm. Density", &body->m_volatileGas,
		0.0, gasGiant ? 50.0 : 1.225, "%.3f kg/m³", 0);
	if (Draw::UndoHelper("Edit Atmosphere Density", undo))
		AddUndoSingleValueClosure(undo, &body->m_volatileGas, updateBodyDerived);

	Draw::HelpMarker("Atmospheric density at the body's nominal surface.\n"
		"Earth has a density of 1.225kg/m³ at normal surface pressure and temperature.");

	Draw::InputFixedSlider("Atm. Oxygen", &body->m_atmosOxidizing);
	if (Draw::UndoHelper("Edit Atmosphere Oxygen", undo))
		AddUndoSingleValue(undo, &body->m_atmosOxidizing);

	Draw::HelpMarker("Proportion of oxidizing elements in atmosphere, e.g. CO², O².\n"
		"Oxidizing elements are a hallmark of outdoor worlds and are needed for human life to survive.");

	Draw::InputFixedSlider("Ocean Coverage", &body->m_volatileLiquid);
	if (Draw::UndoHelper("Edit Ocean Coverage", undo))
		AddUndoSingleValue(undo, &body->m_volatileLiquid);

	Draw::InputFixedSlider("Ice Coverage", &body->m_volatileIces);
	if (Draw::UndoHelper("Edit Ice Coverage", undo))
		AddUndoSingleValue(undo, &body->m_volatileIces);

	// TODO: unused by other code
	// Draw::InputFixedSlider("Human Activity", &body->m_humanActivity);
	// if (Draw::UndoHelper("Edit Human Activity", undo))
	// 	AddUndoSingleValue(undo, &body->m_humanActivity);

	Draw::InputFixedSlider("Life", &body->m_life);
	if (Draw::UndoHelper("Edit Life", undo))
		AddUndoSingleValue(undo, &body->m_life);

	if (Draw::DerivedValues("Surface Parameters")) {
		ImGui::BeginDisabled();

		if (bodyChanged)
			body->SetAtmFromParameters();

		double pressure_p0 = body->GetAtmSurfacePressure();
		ImGui::InputDouble("Surface Pressure", &pressure_p0, 0.0, 0.0, "%.4f atm");

		double atmRadius = body->GetAtmRadius() / 1000.0;
		ImGui::InputDouble("Atmosphere Height", &atmRadius, 0.0, 0.0, "%.2f km");

		bool scoopable = body->IsScoopable();
		ImGui::Checkbox("Is Scoopable", &scoopable);

		ImGui::EndDisabled();
	}

	EditEconomicProperties(body, undo);
}

void SystemBody::EditorAPI::GenerateDerivedStats(SystemBody *body, Random &rng, UndoSystem *undo)
{
	undo->BeginEntry("Generate Body Parameters");

	// Back up all potentially-modified body variables
	AddUndoSingleValue(undo, &body->m_mass);
	AddUndoSingleValue(undo, &body->m_type);
	AddUndoSingleValue(undo, &body->m_radius);
	AddUndoSingleValue(undo, &body->m_averageTemp);

	AddUndoSingleValue(undo, &body->m_axialTilt);
	AddUndoSingleValue(undo, &body->m_rotationPeriod);

	AddUndoSingleValue(undo, &body->m_metallicity);
	AddUndoSingleValue(undo, &body->m_volcanicity);
	AddUndoSingleValue(undo, &body->m_volatileGas);
	AddUndoSingleValue(undo, &body->m_atmosOxidizing);
	AddUndoSingleValue(undo, &body->m_volatileLiquid);
	AddUndoSingleValue(undo, &body->m_volatileIces);
	// AddUndoSingleValue(undo, &body->m_humanActivity);
	AddUndoSingleValue(undo, &body->m_life);

	StarSystemRandomGenerator().PickPlanetType(body, rng);

	undo->EndEntry();
}
