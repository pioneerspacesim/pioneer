#include "GameConfig.h"
#include "KeyBindings.h"

GameConfig::GameConfig(const std::string &filename) : IniConfig(filename)
{
	// set defaults
	(*this)["Lang"] = "English";
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
	(*this)["FOV_Vertical"] = "65";
	(*this)["MasterVolume"] = "0.8";
	(*this)["MusicVolume"] = "0.8";
	(*this)["MasterMuted"] = "0";
	(*this)["SfxMuted"] = "0";
	(*this)["MusicMuted"] = "0";
	(*this)["SectorViewXRotation"] = "-10.0";
	(*this)["SectorViewZRotation"] = "0";
	(*this)["SectorViewZoom"] = "2.0";
	(*this)["MaxPhysicsCyclesPerRender"] = "4";
	(*this)["AntiAliasingMode"] = "2";
	(*this)["JoystickDeadzone"] = "0.1";

#ifdef _WIN32
	(*this)["RedirectStdio"] = "1";
#else
	(*this)["RedirectStdio"] = "0";
#endif

	KeyBindings::SetDefaults();

	Load();

	KeyBindings::OnKeyBindingsChanged();
}
