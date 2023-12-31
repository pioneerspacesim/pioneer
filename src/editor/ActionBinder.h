// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "imgui/imgui.h"

#include <sigc++/sigc++.h>

#include <map>
#include <string>
#include <variant>

namespace Editor {

	// Represents an action the user can execute within an editor context
	// that should be associated with some window-global shortcut.
	// The predicate will be evaluated to determine if the entry is enabled.
	struct ActionEntry {
		template<typename Functor>
		ActionEntry(std::string_view label, ImGuiKeyChord shortcut, Functor f) :
			label(label),
			shortcut(shortcut),
			action(f)
		{}

		template<typename Predicate, typename Functor>
		ActionEntry(std::string_view label, ImGuiKeyChord shortcut, Predicate p, Functor f) :
			label(label),
			shortcut(shortcut),
			predicate(p),
			action(f)
		{}

		template<typename Functor>
		ActionEntry(std::string_view label, const char *icon, ImGuiKeyChord shortcut, Functor f) :
			label(label),
			fontIcon(icon),
			shortcut(shortcut),
			action(f)
		{}

		template<typename Predicate, typename Functor>
		ActionEntry(std::string_view label, const char *icon, ImGuiKeyChord shortcut, Predicate p, Functor f) :
			label(label),
			fontIcon(icon),
			shortcut(shortcut),
			predicate(p),
			action(f)
		{}

		std::string label;
		const char *fontIcon;
		ImGuiKeyChord shortcut;

		sigc::slot<bool ()> predicate;
		sigc::slot<void ()> action;
	};

	class ActionBinder {
	public:
		ActionBinder();
		~ActionBinder();

		struct Group;

		using GroupEntry = std::variant<Group *, ActionEntry *>;
		struct Group {
			Group(std::string_view name, bool menu) :
				label(std::string(name)),
				isMenu(menu)
			{}

			std::string label;
			std::vector<GroupEntry> entries;
			bool isMenu;
		};

		// process all actions and determine if their shortcuts are activated
		void Update();

		// Draw debug window displaying all registered actions
		void DrawOverview(const char *title, bool *pOpen = nullptr);

		// Draw all groups registered in this ActionBinder as a main menu bar
		void DrawMenuBar();

		// draw the GroupEntry named by 'id' in the context of an existing
		// dropdown menu (i.e. do not submit a top-level BeginMenu)
		void DrawGroup(std::string id) { DrawMenuInternal(m_groupStorage.at(id), false); }

		// draw the GroupEntry named by 'id' as a submenu (with a top-level BeginMenu)
		void DrawMenu(std::string id) { DrawMenuInternal(m_groupStorage.at(id), true); }

		// draw the GroupEntry named by 'id' in the context of a button toolbar
		// void DrawToolbar(std::string id)

		static std::string FormatShortcut(ImGuiKeyChord shortcut);

		// Begin a group or menu with the given ID. ID is a qualified domain
		// name relative to the current ID stack.
		ActionBinder &BeginGroup(std::string id) { BeginInternal(id, false); return *this; }
		ActionBinder &BeginMenu(std::string id) { BeginInternal(id, true); return *this; }
		void EndGroup() { EndInternal(); }
		void EndMenu() { EndInternal(); }

		// Add the given action to the currently open group. Fails if no group
		// is open. The action can be referenced by the fully-qualified id
		// 'group-id[.group-id[...]].action-id'.
		ActionBinder &AddAction(std::string id, ActionEntry &&entry) { AddInternal(id, std::move(entry)); return *this; }

		std::vector<std::string> &GetGroups() { return m_topLevelGroups; }

		ActionEntry *GetAction(std::string id);
		void TriggerAction(std::string id);

	private:
		void BeginInternal(std::string id, bool menu);
		void EndInternal();

		void AddInternal(std::string id, ActionEntry &&entry);

		void DrawMenuInternal(Group &group, bool menuHeading);

		void DrawGroupOverviewEntry(Group &group);

		std::map<std::string, ActionEntry> m_actionStorage;
		std::map<std::string, Group> m_groupStorage;

		std::vector<std::string> m_topLevelGroups;

		std::vector<std::pair<std::string, Group *>> m_activeGroupStack;
	};

}
