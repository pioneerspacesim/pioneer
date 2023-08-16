// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "GalaxyEditAPI.h"

#include "editor/UndoSystem.h"
#include "galaxy/StarSystem.h"
#include "galaxy/SystemBody.h"

namespace Editor::SystemEditorHelpers {

	class UndoManageStarSystemBody : public UndoStep {
	public:
		UndoManageStarSystemBody(StarSystem *system, SystemBody *add, SystemBody *rem = nullptr, bool apply = false) :
			m_system(system),
			m_addBody(add),
			m_remBody(rem)
		{
			if (apply)
				Swap();
		}

		void Swap() override {
			if (m_addBody)
				StarSystem::EditorAPI::AddBody(m_system, m_addBody.Get());
			if (m_remBody)
				StarSystem::EditorAPI::RemoveBody(m_system, m_remBody.Get());
			std::swap(m_addBody, m_remBody);
		}

	private:
		StarSystem *m_system;
		RefCountedPtr<SystemBody> m_addBody;
		RefCountedPtr<SystemBody> m_remBody;
	};

	// UndoStep helper to handle adding or deleting a child SystemBody from a parent
	class UndoAddRemoveChildBody : public UndoStep {
	public:
		UndoAddRemoveChildBody(SystemBody *parent, SystemBody *add, size_t idx) :
			m_parent(parent),
			m_add(add),
			m_idx(idx)
		{
			Swap();
		}

		UndoAddRemoveChildBody(SystemBody *parent, SystemBody *add) :
			m_parent(parent),
			m_add(add),
			m_idx(-1)
		{
			Swap();
		}

		UndoAddRemoveChildBody(SystemBody *parent, size_t idx) :
			m_parent(parent),
			m_add(nullptr),
			m_idx(idx)
		{
			Swap();
		}

		void Swap() override {
			if (m_add) {
				SystemBody::EditorAPI::AddChild(m_parent, m_add.Get(), m_idx);
				m_add.Reset();
			} else {
				m_add.Reset(SystemBody::EditorAPI::RemoveChild(m_parent, m_idx));
			}
		}

	private:
		SystemBody *m_parent;
		RefCountedPtr<SystemBody> m_add;
		size_t m_idx;
	};

} // namespace Editor
