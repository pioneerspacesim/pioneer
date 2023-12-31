// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"
#include <memory>

class Galaxy;
class GameConfig;
class SystemPath;
namespace Input { class Manager; }
namespace Graphics { class Renderer; }
namespace PiGui { class Instance; }

struct SectorMapContext {
public:

	RefCountedPtr<Galaxy> galaxy;
	Input::Manager *input;
	GameConfig *config;
	Graphics::Renderer *renderer;
	PiGui::Instance *pigui;

	using DisplayMode = uint32_t;
	enum DisplayModes : DisplayMode {
		DEFAULT =           0,
		ALWAYS =       1 << 0,
		HIDE_LABEL =   1 << 1,
		SHADOW_LABEL = 1 << 2
	};
	class Callbacks {
	public:
		using DisplayModes = SectorMapContext::DisplayModes;
		virtual ~Callbacks(){}
		virtual void OnClickLabel(const SystemPath &clickedLabel) = 0;
		// custom filter - called for each displayed system
		virtual DisplayMode GetDisplayMode(const SystemPath &system) = 0;
	};

	std::unique_ptr<Callbacks> callbacks;
};
