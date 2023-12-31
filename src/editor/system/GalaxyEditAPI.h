// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "galaxy/StarSystem.h"
#include "galaxy/SystemBody.h"

class LuaNameGen;
class FactionsDatabase;

namespace Editor {
	class UndoSystem;

	struct CustomSystemInfo {
		enum ExplorationState {
			EXPLORE_Random = 0,
			EXPLORE_ExploredAtStart,
			EXPLORE_Unexplored,
		};

		ExplorationState explored = EXPLORE_Random;
		bool randomLawlessness = true;
		bool randomFaction = true;

		std::string faction;
		std::string comment;
	};
}

class StarSystem::EditorAPI {
public:
	static void RemoveFromCache(StarSystem *system);

	static SystemBody *NewBody(StarSystem *system);
	static SystemBody *NewBodyAround(StarSystem *system, Random &rng, SystemBody *primary, size_t idx);

	static void AddBody(StarSystem *system, SystemBody *body, size_t idx = -1);
	static void RemoveBody(StarSystem *system, SystemBody *body);

	// Set body index from hierarchy order
	static void ReorderBodyIndex(StarSystem *system);
	// Set body hierarchy from index order
	static void ReorderBodyHierarchy(StarSystem *system);

	// Sort the bodies in the system based on semi-major axis
	static void SortBodyHierarchy(StarSystem *system, Editor::UndoSystem *undo);

	static void EditName(StarSystem *system, Random &rng, Editor::UndoSystem *undo);
	static void EditProperties(StarSystem *system, Editor::CustomSystemInfo &custom, FactionsDatabase *factions, Editor::UndoSystem *undo);
};

class SystemBody::EditorAPI {
public:
	static void GenerateDefaultName(SystemBody *body);
	static void GenerateCustomName(SystemBody *body, Random &rng);

	static void AddChild(SystemBody *parent, SystemBody *child, size_t idx = -1);
	static SystemBody *RemoveChild(SystemBody *parent, size_t idx = -1);
	static size_t GetIndexInParent(SystemBody *body);

	static void EditOrbitalParameters(SystemBody *body, Editor::UndoSystem *undo);
	static void EditEconomicProperties(SystemBody *body, Editor::UndoSystem *undo);
	static void EditStarportProperties(SystemBody *body, Editor::UndoSystem *undo);
	static void EditBodyName(SystemBody *body, Random &rng, LuaNameGen *nameGen, Editor::UndoSystem *undo);
	static void EditProperties(SystemBody *body, Random &rng, Editor::UndoSystem *undo);

	static void GenerateDerivedStats(SystemBody *body, Random &rng, Editor::UndoSystem *undo);
};
