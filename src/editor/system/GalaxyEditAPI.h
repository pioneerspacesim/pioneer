// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "galaxy/StarSystem.h"

namespace Editor {
	class UndoSystem;
}

class StarSystem::EditorAPI {
public:
	static void ExportToLua(FILE *f, StarSystem *system, Galaxy *galaxy);

	static SystemBody *NewBody(StarSystem *system);
	static SystemBody *NewBodyAround(StarSystem *system, Random &rng, SystemBody *primary, size_t idx);

	static void AddBody(StarSystem *system, SystemBody *body, size_t idx = -1);
	static void RemoveBody(StarSystem *system, SystemBody *body);

	static void ReorderBodyIndex(StarSystem *system);

	static void EditName(StarSystem *system, Random &rng, Editor::UndoSystem *undo);
	static void EditProperties(StarSystem *system, Editor::UndoSystem *undo);
};

class SystemBody::EditorAPI {
public:
	// Return a list of star types in the system; expects to be passed the root body
	static std::string GetStarTypes(SystemBody *body);
	// NOTE: duplicated from StarSystem.cpp
	static std::string ExportToLua(FILE *f, SystemBody *body);

	static void AddChild(SystemBody *parent, SystemBody *child, size_t idx = -1);
	static SystemBody *RemoveChild(SystemBody *parent, size_t idx = -1);
	static size_t GetIndexInParent(SystemBody *body);

	static void EditOrbitalParameters(SystemBody *body, Editor::UndoSystem *undo);
	static void EditEconomicProperties(SystemBody *body, Editor::UndoSystem *undo);
	static void EditStarportProperties(SystemBody *body, Editor::UndoSystem *undo);
	static void EditProperties(SystemBody *body, Random &rng, Editor::UndoSystem *undo);

	static void GenerateDerivedStats(SystemBody *body, Random &rng, Editor::UndoSystem *undo);
};
