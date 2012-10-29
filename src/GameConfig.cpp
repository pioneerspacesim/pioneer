// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GameConfig.h"
#include "KeyBindings.h"

GameConfig::GameConfig(const std::string &filename) : IniConfig(filename)
{
	// set defaults
	m_map["Lang"] = "English";
	m_map["DisableShaders"] = "0";
	m_map["DisableSound"] = "0";
	m_map["StartFullscreen"] = "0";
	m_map["ScrWidth"] = "800";
	m_map["ScrHeight"] = "600";
	m_map["DetailCities"] = "1";
	m_map["DetailPlanets"] = "1";
	m_map["SfxVolume"] = "0.8";
	m_map["EnableJoystick"] = "1";
	m_map["InvertMouseY"] = "0";
	m_map["FOVVertical"] = "65";
	m_map["DisplayNavTunnel"] = "0";
	m_map["MasterVolume"] = "0.8";
	m_map["MusicVolume"] = "0.8";
	m_map["MasterMuted"] = "0";
	m_map["SfxMuted"] = "0";
	m_map["MusicMuted"] = "0";
	m_map["SectorViewXRotation"] = "-10.0";
	m_map["SectorViewZRotation"] = "0";
	m_map["SectorViewZoom"] = "2.0";
	m_map["MaxPhysicsCyclesPerRender"] = "4";
	m_map["AntiAliasingMode"] = "2";
	m_map["JoystickDeadzone"] = "0.1";
	m_map["DefaultLowThrustPower"] = "0.25";
	m_map["VSync"] = "0";
	m_map["UseTextureCompression"] = "0";
	m_map["CockpitCamera"] = "1";

#ifdef _WIN32
	m_map["RedirectStdio"] = "1";
#else
	m_map["RedirectStdio"] = "0";
#endif

	Load();
}
