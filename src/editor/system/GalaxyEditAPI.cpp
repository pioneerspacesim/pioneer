// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
#include "GalaxyEditAPI.h"


#include "EditorIcons.h"
#include "SystemEditorHelpers.h"

#include "core/Log.h"
#include "core/macros.h"
#include "editor/UndoStepType.h"
#include "editor/EditorDraw.h"

#include "EnumStrings.h"
#include "galaxy/Sector.h"
#include "galaxy/Galaxy.h"
#include "galaxy/NameGenerator.h"
#include "galaxy/StarSystemGenerator.h"

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "lua/LuaNameGen.h"
#include "system/SystemBodyUndo.h"

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

	static const char *explored_labels[] = {
		"Randomly Generated",
		"Explored at Start",
		"Unexplored",
	};
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

void StarSystem::EditorAPI::RemoveFromCache(StarSystem *system)
{
	if (system->m_cache) {
		system->m_cache->RemoveFromAttic(system->GetPath());
		system->m_cache = nullptr;
	}
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
	std::vector<std::pair<SystemBody *, size_t>> orderStack {
		{ system->GetRootBody().Get(), 0 }
	};

	while (!orderStack.empty()) {
		auto &pair = orderStack.back();
		SystemBody *body = pair.first;

		if (pair.second == 0)
			// Set body index from hierarchy order
			body->m_path.bodyIndex = index++;

		if (pair.second < body->GetNumChildren())
			orderStack.push_back({ body->GetChildren()[pair.second++], 0 });
		else
			orderStack.pop_back();
	}

	std::sort(system->m_bodies.begin(), system->m_bodies.end(), [](auto a, auto b) {
		return a->m_path.bodyIndex < b->m_path.bodyIndex;
	});
}

void StarSystem::EditorAPI::ReorderBodyHierarchy(StarSystem *system)
{
	std::vector<std::pair<SystemBody *, size_t>> orderStack {
		{ system->GetRootBody().Get(), 0 }
	};

	while (!orderStack.empty()) {
		auto &pair = orderStack.back();
		SystemBody *body = pair.first;

		if (pair.second == 0)
			// Sort body order from index
			std::sort(body->m_children.begin(), body->m_children.end(),
				[](auto *a, auto *b) { return a->m_path.bodyIndex < b->m_path.bodyIndex; });

		if (pair.second < body->GetNumChildren())
			orderStack.push_back({ body->GetChildren()[pair.second++], 0 });
		else
			orderStack.pop_back();
	}

	std::sort(system->m_bodies.begin(), system->m_bodies.end(), [](auto a, auto b) {
		return a->m_path.bodyIndex < b->m_path.bodyIndex;
	});
}

void StarSystem::EditorAPI::SortBodyHierarchy(StarSystem *system, UndoSystem *undo)
{
	size_t index = 0;
	std::vector<std::pair<SystemBody *, size_t>> orderStack {
		{ system->GetRootBody().Get(), 0 }
	};

	undo->AddUndoStep<SystemEditorUndo::SortStarSystemBodies>(system, false);

	while (!orderStack.empty()) {
		auto &pair = orderStack.back();
		SystemBody *body = pair.first;

		if (pair.second == 0) {

			std::stable_sort(body->m_children.begin(), body->m_children.end(),
				[](auto *a, auto *b) { return a->m_semiMajorAxis < b->m_semiMajorAxis; });

			for (SystemBody *body : body->m_children)
				AddUndoSingleValue(undo, &body->m_path.bodyIndex);

			body->m_path.bodyIndex = index++;

		}

		if (pair.second < body->GetNumChildren()) {
			orderStack.push_back({ body->GetChildren()[pair.second++], 0 });
		} else {
			orderStack.pop_back();
		}
	}

	std::sort(system->m_bodies.begin(), system->m_bodies.end(), [](auto a, auto b) {
		return a->m_path.bodyIndex < b->m_path.bodyIndex;
	});

	undo->AddUndoStep<SystemEditorUndo::SortStarSystemBodies>(system, true);
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

	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted("Other Names");

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x * 2.f, 0.f);
	ImGui::Button("+", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
	if (Draw::UndoHelper("Add System Other Name", undo))
		AddUndoVectorInsert(undo, &system->m_other_names, "");

	float window_height = ImGui::GetFrameHeightWithSpacing() * std::min(size_t(4), system->m_other_names.size()) + ImGui::GetStyle().WindowPadding.y * 2.f;

	if (system->m_other_names.size() > 0) {
		ImGui::BeginChild("##Other Names", ImVec2(0, window_height), true);

		for (size_t idx = 0; idx < system->m_other_names.size(); idx++) {

			ImGui::PushID(idx);

			if (ImGui::Button("-")) {
				undo->BeginEntry("Remove System Other Name");
				AddUndoVectorErase(undo, &system->m_other_names, idx);
				undo->EndEntry();

				ImGui::PopID();
				continue;
			}

			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText("##name", &system->m_other_names[idx]);

			if (Draw::UndoHelper("Edit System Other Name", undo))
				AddUndoVectorSingleValue(undo, &system->m_other_names, idx);

			ImGui::PopID();

		}
		ImGui::EndChild();

		ImGui::Spacing();
	}

	ImGui::SeparatorText("Short Description");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputText("##Short Description", &system->m_shortDesc);
	if (Draw::UndoHelper("Edit System Short Description", undo))
		AddUndoSingleValue(undo, &system->m_shortDesc);

	ImGui::SeparatorText("Long Description");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputTextMultiline("##Long Description", &system->m_longDesc, ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 5.f + ImGui::GetStyle().WindowPadding.y * 2.f));
	if (Draw::UndoHelper("Edit System Long Description", undo))
		AddUndoSingleValue(undo, &system->m_longDesc);
}

void StarSystem::EditorAPI::EditProperties(StarSystem *system, CustomSystemInfo &custom, FactionsDatabase *factions, UndoSystem *undo)
{
	ImGui::SeparatorText("Generation Parameters");

	ImGui::InputInt("Seed", reinterpret_cast<int *>(&system->m_seed));
	if (Draw::UndoHelper("Edit Seed", undo))
		AddUndoSingleValue(undo, &system->m_seed);

	bool comboOpen = Draw::ComboUndoHelper("Edit System Exploration State", "Explored State", explored_labels[custom.explored], undo);
	if (comboOpen) {
		if (ImGui::IsWindowAppearing()) {
			AddUndoSingleValue(undo, &custom.explored);
		}

		for (size_t idx = 0; idx < COUNTOF(explored_labels); idx++) {
			if (ImGui::Selectable(explored_labels[idx], idx == custom.explored))
				custom.explored = CustomSystemInfo::ExplorationState(idx);
		}

		ImGui::EndCombo();
	}

	if (Draw::LayoutHorizontal("Sector Path", 3, ImGui::GetFontSize())) {
		ImGui::InputInt("X", &system->m_path.sectorX, 0, 0);
		ImGui::InputInt("Y", &system->m_path.sectorY, 0, 0);
		ImGui::InputInt("Z", &system->m_path.sectorZ, 0, 0);

		Draw::EndLayout();
	}

	if (Draw::UndoHelper("Edit System Sector", undo))
		AddUndoSingleValue(undo, &system->m_path);

	if (Draw::LayoutHorizontal("Position in Sector", 3, ImGui::GetFontSize())) {
		ImGui::SliderFloat("X", &system->m_pos.x, 0.f, 8.f, "%.3f ly", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Y", &system->m_pos.y, 0.f, 8.f, "%.3f ly", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Z", &system->m_pos.z, 0.f, 8.f, "%.3f ly", ImGuiSliderFlags_AlwaysClamp);

		Draw::EndLayout();
	}

	if (Draw::UndoHelper("Edit System Position", undo))
		AddUndoSingleValue(undo, &system->m_pos);

	ImGui::SeparatorText("Legal Parameters");

	Draw::EditEnum("Edit System Government", "Government", "PolitGovType",
		reinterpret_cast<int *>(&system->m_polit.govType), Polit::GovType::GOV_MAX - 1, undo);

	ImGui::Checkbox("Random Faction", &custom.randomFaction);
	if (Draw::UndoHelper("Edit Faction", undo))
		AddUndoSingleValue(undo, &custom.randomFaction);

	ImGui::BeginDisabled(custom.randomFaction);

	if (Draw::ComboUndoHelper("Edit Faction", "Faction", custom.faction.c_str(), undo)) {
		if (ImGui::IsWindowAppearing())
			AddUndoSingleValue(undo, &custom.faction);

		for (size_t factionIdx = 0; factionIdx < factions->GetNumFactions(); factionIdx++) {
			const Faction *fac = factions->GetFaction(factionIdx);

			if (ImGui::Selectable(fac->name.c_str(), fac->name == custom.faction))
				custom.faction = fac->name;
		}

		ImGui::EndCombo();
	}

	ImGui::EndDisabled();

	ImGui::Checkbox("Random Lawlessness", &custom.randomLawlessness);
	if (Draw::UndoHelper("Edit Lawlessness", undo))
		AddUndoSingleValue(undo, &custom.randomLawlessness);

	ImGui::BeginDisabled(custom.randomLawlessness);

	Draw::InputFixedSlider("Lawlessness", &system->m_polit.lawlessness);
	if (Draw::UndoHelper("Edit System Lawlessness", undo))
		AddUndoSingleValue(undo, &system->m_polit.lawlessness);

	ImGui::EndDisabled();

	ImGui::SeparatorText("Author Comments");

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputTextMultiline("##Comment", &custom.comment, ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 5.f + ImGui::GetStyle().WindowPadding.y * 2.f));

	if (Draw::UndoHelper("Edit Comments", undo))
		AddUndoSingleValue(undo, &custom.comment);

}

// ─── SystemBody::EditorAPI ───────────────────────────────────────────────────

void SystemBody::EditorAPI::GenerateDefaultName(SystemBody *body)
{
	SystemBody *parent = body->GetParent();

	// We're the root body, should probably be named after the system
	if (!parent) {
		body->m_name = body->GetStarSystem()->GetName();
		return;
	}

	// Starports get a consistent default 'identifier' name
	if (body->GetSuperType() == SUPERTYPE_STARPORT) {
		Random rand({ body->GetSeed(), UNIVERSE_SEED });

		char ident_1 = rand.Int32('A', 'Z');
		char ident_2 = rand.Int32('A', 'Z');

		body->m_name = fmt::format("{} {}{}-{:04d}",
			body->GetType() == TYPE_STARPORT_ORBITAL ? "Orbital" : "Port",
			ident_1, ident_2, rand.Int32(10000));
		return;
	}

	// Other bodies get a "hierarchy" name

	size_t idx = 0;
	for (SystemBody *child : parent->m_children) {
		if (child == body)
			break;
		if (child->GetSuperType() != SUPERTYPE_STARPORT)
			idx++;
	}

	if (parent->GetSuperType() <= SystemBody::SUPERTYPE_STAR) {
		if (idx <= 26)
			body->m_name = fmt::format("{} {}", parent->GetName(), char('a' + idx));
		else
			body->m_name = fmt::format("{} {}{}", parent->GetName(), char('a' + idx / 26), char('a' + idx % 26));
	} else {
		body->m_name = fmt::format("{} {}", parent->GetName(), 1 + idx);
	}
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

			ImGui::InputDouble("Orbital Vel. (AP)", &orbit_vel_ap, 0.0, 0.0, "%.2f km/s");
			ImGui::InputDouble("Orbital Vel. (PE)", &orbit_vel_pe, 0.0, 0.0, "%.2f km/s");
		}

		double escape_velocity = body->CalcEscapeVelocity();
		ImGui::InputDouble("Escape Velocity", &escape_velocity, 0.0, 0.0, "%0.2f km/s");

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

	// TODO: system generation currently ignores these fields of a system body
	// and overwrites them with randomly-rolled values.
	ImGui::BeginDisabled();

	ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
	ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1.0), "These fields are currently overwritten when the system is loaded.");
	ImGui::PopTextWrapPos();
	ImGui::Spacing();

	ImGui::InputFixed("Population", &body->m_population);
	if (Draw::UndoHelper("Edit Population", undo))
		AddUndoSingleValue(undo, &body->m_population);

	ImGui::InputFixed("Agricultural Activity", &body->m_agricultural);
	if (Draw::UndoHelper("Edit Agricultural Activity", undo))
		AddUndoSingleValue(undo, &body->m_agricultural);

	ImGui::EndDisabled();
}

void SystemBody::EditorAPI::EditStarportProperties(SystemBody *body, UndoSystem *undo)
{
	bool orbitChanged = false;

	if (body->GetType() == TYPE_STARPORT_SURFACE) {
		ImGui::SeparatorText("Surface Parameters");

		orbitChanged |= Draw::InputFixedDegrees("Latitude", &body->m_inclination);
		if (Draw::UndoHelper("Edit Latitude", undo))
			AddUndoSingleValueClosure(undo, &body->m_inclination, [=](){ body->SetOrbitFromParameters(); });

		orbitChanged |= Draw::InputFixedDegrees("Longitude", &body->m_orbitalOffset);
		if (Draw::UndoHelper("Edit Longitude", undo))
			AddUndoSingleValueClosure(undo, &body->m_orbitalOffset, [=](){ body->SetOrbitFromParameters(); });

		if (orbitChanged)
			body->SetOrbitFromParameters();

	} else {
		EditOrbitalParameters(body, undo);
	}

	EditEconomicProperties(body, undo);

	ImGui::SeparatorText("Misc. Properties");

	ImGui::InputText("Model Name", &body->m_spaceStationType);
	if (Draw::UndoHelper("Edit Station Model Name", undo))
		AddUndoSingleValue(undo, &body->m_spaceStationType);

	Draw::HelpMarker("Model name (without extension) to use for this starport.\nA random model is chosen if not specified.");
}

void SystemBody::EditorAPI::EditBodyName(SystemBody *body, Random &rng, LuaNameGen *nameGen, UndoSystem *undo)
{
	ImGui::BeginGroup();
	ImGui::InputText("Name", &body->m_name);

	if (ImGui::Button(EICON_RESET " Default Name"))
		GenerateDefaultName(body);

	ImGui::SameLine();

	// allocate a new random generator here so it can be pushed to lua
	RefCountedPtr<Random> rand { new Random({ rng.Int32() }) };

	if (ImGui::Button(EICON_RANDOM " Random Name"))
		body->m_name = nameGen->BodyName(body, rand);

	ImGui::EndGroup();

	if (Draw::UndoHelper("Edit Name", undo))
		AddUndoSingleValue(undo, &body->m_name);
}

void SystemBody::EditorAPI::EditProperties(SystemBody *body, Random &rng, UndoSystem *undo)
{
	bool isStar = body->GetSuperType() <= SUPERTYPE_STAR;

	bool bodyChanged = false;
	auto updateBodyDerived = [=]() {
		body->SetAtmFromParameters();
	};

	Draw::EditEnum("Edit Body Type", "Body Type", "BodyType", reinterpret_cast<int *>(&body->m_type), BodyType::TYPE_MAX, undo);

	ImGui::InputInt("Seed", reinterpret_cast<int *>(&body->m_seed));
	if (Draw::UndoHelper("Edit Seed", undo))
		AddUndoSingleValue(undo, &body->m_seed);

	if (body->GetSuperType() < SUPERTYPE_STARPORT) {

		if ((!isStar || body->GetType() == TYPE_BROWN_DWARF) && ImGui::Button(EICON_RANDOM " Body Stats")) {
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

		double surfaceGrav = body->CalcSurfaceGravity() / 9.80665; // compute g-force
		ImGui::InputDouble("Surface Gravity", &surfaceGrav, 0, 0, "%.4f g");

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
		0.0, gasGiant ? 2.0 : 1.225, "%.3f kg/m³", 0);
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

	ImGui::InputText("HMap Path", &body->m_heightMapFilename);
	if (Draw::UndoHelper("Edit Heightmap Path", undo))
		AddUndoSingleValue(undo, &body->m_heightMapFilename);

	Draw::HelpMarker("Path to a custom heightmap file for this body, relative to the game's data directory.");

	ImGui::SliderInt("HMap Fractal", reinterpret_cast<int *>(&body->m_heightMapFractal), 0, 1, "%d", ImGuiSliderFlags_AlwaysClamp);
	if (Draw::UndoHelper("Edit Heightmap Fractal", undo))
		AddUndoSingleValue(undo, &body->m_heightMapFractal);

	Draw::HelpMarker("Fractal type index for use with a custom heightmap file.");

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
