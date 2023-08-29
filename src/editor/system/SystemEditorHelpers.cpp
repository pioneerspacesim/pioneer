// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditorHelpers.h"
#include "core/macros.h"
#include "gameconsts.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

enum DistanceUnits {
	DISTANCE_AU, // Astronomical units
	DISTANCE_LS, // Light-seconds
	DISTANCE_KM, // Kilometers
};

const char *distance_labels[] = {
	"AU",
	"ls",
	"km"
};

const char *distance_formats[] = {
	"%.4f AU",
	"%.4f ls",
	"%.2f km"
};

const double distance_multipliers[] = {
	1.0,
	499.00478,
	AU / 1000.0
};

enum MassUnits {
	MASS_SOLS,   // Solar masses
	MASS_EARTH,  // Earth masses
	MASS_MT,     // Kilograms x 1,000,000,000
};

const char *mass_labels[] = {
	"Solar Masses",
	"Earth Masses",
	"Megatonnes"
};

const char *mass_formats[] = {
	"%.4f Sol",
	"%.4f Earth",
	"%.2f Mt"
};

const double KG_TO_MT = 1000000000;

const double SOL_MASS_MT = SOL_MASS / KG_TO_MT;
const double EARTH_MASS_MT = EARTH_MASS / KG_TO_MT;
const double SOL_TO_EARTH_MASS = SOL_MASS / EARTH_MASS;

enum RadiusUnits {
	RADIUS_SOLS,   // Solar masses
	RADIUS_EARTH,  // Earth masses
	RADIUS_KM,     // Kilometers
};

const char *radius_labels[] = {
	"Solar Radii",
	"Earth Radii",
	"km"
};

const char *radius_formats[] = {
	"%.4f Sol",
	"%.4f Earth",
	"%.2f km"
};

const double SOL_RADIUS_KM = SOL_RADIUS / 1000.0;
const double EARTH_RADIUS_KM = EARTH_RADIUS / 1000.0;
const double SOL_TO_EARTH_RADIUS = SOL_RADIUS / EARTH_RADIUS;

void Draw::SubtractItemWidth()
{
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	float used_width = window->DC.CursorPos.x - IM_FLOOR(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - used_width);
}

bool Draw::InputFixedSlider(const char *str, fixed *val, double val_min, double val_max, const char *format, ImGuiInputTextFlags flags)
{
	double val_d = val->ToDouble();

	bool changed = ImGui::SliderScalar(str, ImGuiDataType_Double, &val_d, &val_min, &val_max, format, ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
	// delay one frame before writing back the value for the undo system to push a value
	if (changed && !ImGui::IsItemActivated())
		*val = fixed::FromDouble(val_d);

	return changed && !ImGui::IsItemActivated();
}

bool Draw::InputFixedDegrees(const char *str, fixed *val, ImGuiInputTextFlags flags)
{
	double val_d = RAD2DEG(val->ToDouble());
	const double val_min = -360.0;
	const double val_max = 360.0;

	bool changed = ImGui::SliderScalar(str, ImGuiDataType_Double, &val_d, &val_min, &val_max, "%.3f°", ImGuiSliderFlags_NoRoundToFormat);
	// bool changed = ImGui::InputDouble(str, &val_d, 1.0, 10.0, "%.3f°", flags | ImGuiInputTextFlags_EnterReturnsTrue);
	// delay one frame before writing back the value for the undo system to push a value
	if (changed && !ImGui::IsItemActivated())
		*val = fixed::FromDouble(DEG2RAD(val_d));

	return changed && !ImGui::IsItemActivated();
}

bool Draw::InputFixedDistance(const char *str, fixed *val, ImGuiInputTextFlags flags)
{
	ImGuiStyle &style = ImGui::GetStyle();

	ImGui::BeginGroup();
	ImGui::PushID(str);

	ImGuiID unit_type_id = ImGui::GetID("#UnitType");

	int unit_type = DISTANCE_AU;

	double val_d = val->ToDouble();
	if (val_d < 0.0001)
		unit_type = DISTANCE_LS;
	if (val_d < 0.000001)
		unit_type = DISTANCE_KM;

	unit_type = ImGui::GetStateStorage()->GetInt(unit_type_id, unit_type);

	ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
	if (ImGui::Combo("##Unit", &unit_type, distance_labels, COUNTOF(distance_labels))) {
		ImGui::GetStateStorage()->SetInt(unit_type_id, unit_type);
	}

	double val_step = unit_type == DISTANCE_KM ? 1.0 : unit_type == DISTANCE_LS ? 0.1 : 0.01;
	val_d *= distance_multipliers[unit_type];

	ImGui::SameLine(0.f, 1.f);
	Draw::SubtractItemWidth();
	bool changed = ImGui::InputDouble(str, &val_d, val_step, val_step * 10.0, distance_formats[unit_type], flags | ImGuiInputTextFlags_EnterReturnsTrue);
	if (changed)
		*val = fixed::FromDouble(val_d / distance_multipliers[unit_type]);

	ImGui::PopID();
	ImGui::EndGroup();

	return changed;
}

bool Draw::InputFixedMass(const char *str, fixed *val, bool is_solar, ImGuiInputTextFlags flags)
{
	ImGuiStyle &style = ImGui::GetStyle();

	ImGui::BeginGroup();
	ImGui::PushID(str);

	ImGuiID unit_type_id = ImGui::GetID("#UnitType");

	int unit_type = MASS_SOLS;
	if (!is_solar)
		unit_type = MASS_EARTH;

	double val_d = val->ToDouble();
	if (!is_solar && val_d < 0.0000001)
		unit_type = MASS_MT;

	unit_type = ImGui::GetStateStorage()->GetInt(unit_type_id, unit_type);

	ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
	if (ImGui::Combo("##Unit", &unit_type, mass_labels, COUNTOF(mass_labels))) {
		ImGui::GetStateStorage()->SetInt(unit_type_id, unit_type);
	}
	double val_step = 0.01;

	if (is_solar && unit_type != MASS_SOLS)
		val_d *= unit_type == MASS_EARTH ? SOL_TO_EARTH_MASS : SOL_MASS_MT;
	if (!is_solar && unit_type != MASS_EARTH)
		val_d *= unit_type == MASS_SOLS ? (1.0 / SOL_TO_EARTH_MASS) : EARTH_MASS_MT;

	ImGui::SameLine(0.f, 1.f);
	Draw::SubtractItemWidth();
	bool changed = ImGui::InputDouble(str, &val_d, val_step, val_step * 10.0, mass_formats[unit_type], flags | ImGuiInputTextFlags_EnterReturnsTrue);

	if (is_solar && unit_type != MASS_SOLS)
		val_d /= unit_type == MASS_EARTH ? SOL_TO_EARTH_MASS : SOL_MASS_MT;
	if (!is_solar && unit_type != MASS_EARTH)
		val_d /= unit_type == MASS_SOLS ? (1.0 / SOL_TO_EARTH_MASS) : EARTH_MASS_MT;

	if (changed)
		*val = fixed::FromDouble(val_d);

	ImGui::PopID();
	ImGui::EndGroup();

	return changed;
}

bool Draw::InputFixedRadius(const char *str, fixed *val, bool is_solar, ImGuiInputTextFlags flags)
{
	ImGuiStyle &style = ImGui::GetStyle();

	ImGui::BeginGroup();
	ImGui::PushID(str);

	ImGuiID unit_type_id = ImGui::GetID("#UnitType");

	int unit_type = RADIUS_SOLS;
	if (!is_solar)
		unit_type = RADIUS_EARTH;

	double val_d = val->ToDouble();
	if (!is_solar && val_d < 0.1)
		unit_type = RADIUS_KM;

	unit_type = ImGui::GetStateStorage()->GetInt(unit_type_id, unit_type);

	ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
	if (ImGui::Combo("##Unit", &unit_type, radius_labels, COUNTOF(radius_labels))) {
		ImGui::GetStateStorage()->SetInt(unit_type_id, unit_type);
	}
	double val_step = 0.01;

	if (is_solar && unit_type != RADIUS_SOLS)
		val_d *= unit_type == RADIUS_EARTH ? SOL_TO_EARTH_MASS : SOL_RADIUS_KM;
	if (!is_solar && unit_type != RADIUS_EARTH)
		val_d *= unit_type == RADIUS_SOLS ? (1.0 / SOL_TO_EARTH_MASS) : EARTH_RADIUS_KM;

	ImGui::SameLine(0.f, 1.f);
	Draw::SubtractItemWidth();
	bool changed = ImGui::InputDouble(str, &val_d, val_step, val_step * 10.0, radius_formats[unit_type], flags | ImGuiInputTextFlags_EnterReturnsTrue);

	if (is_solar && unit_type != RADIUS_SOLS)
		val_d /= unit_type == RADIUS_EARTH ? SOL_TO_EARTH_MASS : SOL_RADIUS_KM;
	if (!is_solar && unit_type != RADIUS_EARTH)
		val_d /= unit_type == RADIUS_SOLS ? (1.0 / SOL_TO_EARTH_MASS) : EARTH_RADIUS_KM;

	if (changed)
		*val = fixed::FromDouble(val_d);

	ImGui::PopID();
	ImGui::EndGroup();

	return changed;
}
