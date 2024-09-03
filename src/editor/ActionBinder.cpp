// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ActionBinder.h"
#include "core/StringUtils.h"
#include "fmt/core.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <algorithm>

using namespace Editor;

// wrap double-dereference when working with variant of pointer types
template<typename T, typename ...Types>
auto get_if(std::variant<Types...> &pv)
{
	T *ptr = std::get_if<T>(&pv);
	return ptr ? *ptr : nullptr;
}

ActionBinder::ActionBinder()
{
}

ActionBinder::~ActionBinder()
{
}

// static
std::string ActionBinder::FormatShortcut(ImGuiKeyChord shortcut)
{
	char name[24];
	ImGui::GetKeyChordName(shortcut, name, sizeof(name));

	return std::string(name);
}

void ActionBinder::Update()
{
	// Don't process shortcuts while a popup is open
	if (ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId)) {
		for (auto &popup : ImGui::GetCurrentContext()->OpenPopupStack) {
			if (popup.Window && popup.Window->Flags & ImGuiWindowFlags_Modal)
				return;
		}
	}

	for (auto &[id, action] : m_actionStorage) {
		if (!action.shortcut)
			continue;

		if (ImGui::Shortcut(action.shortcut, 0, ImGuiInputFlags_RouteGlobal)) {
			if (action.predicate.empty() || action.predicate())
				action.action();
		}
	}
}

void ActionBinder::DrawGroupOverviewEntry(Group &group)
{
	if (group.isMenu)
		if (!ImGui::TreeNodeEx(group.label.c_str(), ImGuiTreeNodeFlags_FramePadding))
			return;

	for (auto &entry : group.entries) {
		if (auto *action = get_if<ActionEntry *>(entry)) {
			ImGui::TextUnformatted(action->label.c_str());

			if (action->shortcut) {
				ImGui::SameLine(ImGui::CalcItemWidth());
				ImGui::TextUnformatted(FormatShortcut(action->shortcut).c_str());
			}
		} else if (auto *group = get_if<Group *>(entry)) {
			DrawGroupOverviewEntry(*group);
		}
	}

	if (group.isMenu)
		ImGui::TreePop();
}

void ActionBinder::DrawOverview(const char *title, bool *pOpen)
{
	if (ImGui::Begin(title, pOpen)) {
		for (auto &groupId : m_topLevelGroups) {
			DrawGroupOverviewEntry(m_groupStorage.at(groupId));
		}
	}
	ImGui::End();
}

void ActionBinder::DrawMenuBar()
{
	for (auto &groupId : m_topLevelGroups) {
		Group &group = m_groupStorage.at(groupId);

		if (group.isMenu)
			DrawMenuInternal(group, true);
	}
}

void ActionBinder::DrawMenuInternal(Group &group, bool submitMenuItem)
{
	if (group.entries.empty())
		return;

	if (submitMenuItem && !ImGui::BeginMenu(group.label.c_str()))
		return;

	ImGui::PushID(group.label.c_str());

	size_t numActions = 0;
	for (auto &entry : group.entries) {

		if (ActionEntry *action = get_if<ActionEntry *>(entry)) {

			numActions++;

			bool enabled = action->predicate.empty() || action->predicate();
			ImGui::BeginDisabled(!enabled);

			std::string shortcut = action->shortcut ? FormatShortcut(action->shortcut) : "";
			if (ImGui::MenuItem(action->label.c_str(), shortcut.c_str())) {
				action->action();
			}

			ImGui::EndDisabled();

		} else if (Group *subGroup = get_if<Group *>(entry)) {

			if (numActions) {
				numActions = 0;
				ImGui::Separator();
			}

			DrawMenuInternal(*subGroup, subGroup->isMenu);

		}
	}

	ImGui::PopID();

	if (submitMenuItem)
		ImGui::EndMenu();
}

void ActionBinder::BeginInternal(std::string id, bool menu)
{
	for (std::string_view name : SplitString(id, ".")) {

		std::string lookupId = !m_activeGroupStack.empty() ?
			fmt::format("{}.{}", m_activeGroupStack.back().first, name) :
			std::string(name);

		// Create the new group if it doesn't exist
		if (!m_groupStorage.count(lookupId)) {
			m_groupStorage.try_emplace(lookupId, name, menu);

			// nothing on the stack, could be a top-level entry
			if (m_activeGroupStack.empty())
				m_topLevelGroups.push_back(lookupId);
			// add a group entry to our previous group
			else {
				Group *group = m_activeGroupStack.back().second;
				group->entries.emplace_back(&m_groupStorage.at(lookupId));
			}
		}

		m_activeGroupStack.push_back({ lookupId, &m_groupStorage.at(lookupId) });
	}
}

void ActionBinder::EndInternal()
{
	assert(!m_activeGroupStack.empty());

	m_activeGroupStack.pop_back();
}

void ActionBinder::AddInternal(std::string id, ActionEntry &&entry)
{
	if (m_activeGroupStack.empty())
		return;

	Group *group = m_activeGroupStack.back().second;

	std::string qualified_id = fmt::format("{}.{}", m_activeGroupStack.back().first, id);
	auto iter = m_actionStorage.emplace(qualified_id, std::move(entry)).first;

	group->entries.push_back(&iter->second);
}

ActionEntry *ActionBinder::GetAction(std::string id)
{
	if (!m_actionStorage.count(id))
		return nullptr;

	return &m_actionStorage.at(id);
}

void ActionBinder::TriggerAction(std::string id)
{
	ActionEntry *entry = GetAction(id);

	if (entry && (entry->predicate.empty() || entry->predicate()))
		entry->action();
}
