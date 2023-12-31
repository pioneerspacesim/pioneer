// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GameConfig.h"
#include "FileSystem.h"
#include "core/OS.h"

GameConfig::GameConfig(const map_string &override_)
{
	// set defaults
	std::map<std::string, std::string> &map = m_map[""];
	map["Lang"] = OS::GetUserLangCode();
	map["AMD_MESA_HACKS"] = "0";
	map["DisableSound"] = "0";
	map["StartFullscreen"] = "0";
	map["ScrWidth"] = "1280";
	map["ScrHeight"] = "720";
	map["UIScaleFactor"] = "1";
	map["DetailCities"] = "1";
	map["DetailPlanets"] = "1";
	map["SfxVolume"] = "0.8";
	map["EnableJoystick"] = "1";
	map["InvertMouseY"] = "0";
	map["FOVVertical"] = "65";
	map["DisplayNavTunnel"] = "0";
	map["CompactRadar"] = "1";
	map["ConfirmQuit"] = "1";
	map["MasterVolume"] = "0.8";
	map["MusicVolume"] = "0.8";
	map["MasterMuted"] = "0";
	map["SfxMuted"] = "0";
	map["MusicMuted"] = "0";
	map["SectorViewXRotation"] = "-10.0";
	map["SectorViewZRotation"] = "0";
	map["SectorViewZoom"] = "2.0";
	map["MaxPhysicsCyclesPerRender"] = "4";
	map["AntiAliasingMode"] = "2";
	map["JoystickDeadzone"] = "0.2"; // 20% deadzone is common
	map["DefaultLowThrustPower"] = "0.25";
	map["VSync"] = "1";
	map["UseTextureCompression"] = "1";
	map["WorkerThreads"] = "0";
	map["SpeedLines"] = "0";
	map["EnableCockpit"] = "0";
	map["HudTrails"] = "0";
	map["EnableServerAgent"] = "0";
	map["AmountOfBackgroundStars"] = "0.25";
	map["StarFieldStarSizeFactor"] = "0.7";
	map["UseAnisotropicFiltering"] = "0";
	map["RendererName"] = "Opengl 3.x"; // default to our best renderer
	map["EnableGLDebug"] = "0";
	map["EnableGPUJobs"] = "1";
	map["GL3ForwardCompatible"] = "1";
	map["LogVerbose"] = "1";
	map["ProfileSlowFrames"] = "0";
	map["ProfilerZoneOutput"] = "0";
	map["CameraSmoothing"] = "0";
	map["AimingSensitivity"] = "1.0";

	Read(FileSystem::userFiles, "config.ini");

	for (auto i = override_.begin(); i != override_.end(); ++i) {
		const std::string &key = (*i).first;
		const std::string &val = (*i).second;
		map[key] = val;
	}
}
