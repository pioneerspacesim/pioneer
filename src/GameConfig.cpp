#include "GameConfig.h"
#include "KeyBindings.h"

GameConfig::GameConfig(const std::string &filename) : IniConfig(filename)
{
	// set defaults
	(*this)["EnableHDR"] = "0";
	(*this)["DisableShaders"] = "0";
	(*this)["DisableSound"] = "0";
	(*this)["StartFullscreen"] = "0";
	(*this)["ScrWidth"] = "800";
	(*this)["ScrHeight"] = "600";
	(*this)["DetailCities"] = "1";
	(*this)["DetailPlanets"] = "1";
	(*this)["SfxVolume"] = "0.8";
	(*this)["EnableJoystick"] = "1";
	(*this)["InvertMouseY"] = "0";
	(*this)["FOV"] = "83";

	KeyBindings::SetDefaults();

	Load();

	KeyBindings::OnKeyBindingsChanged();
}
