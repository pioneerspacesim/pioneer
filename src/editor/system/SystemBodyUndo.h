// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "GalaxyEditAPI.h"

#include "editor/UndoSystem.h"
#include "galaxy/StarSystem.h"
#include "galaxy/SystemBody.h"

namespace Editor::SystemEditorUndo {

	class ManageStarSystemBody : public UndoStep {
	public:
		ManageStarSystemBody(StarSystem *system, SystemBody *add, SystemBody *rem = nullptr, bool apply = false) :
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

	// Helper to reorder body indexes at the end of an undo / redo operation by adding two undo steps
	class ReorderStarSystemBodies : public UndoStep {
	public:
		ReorderStarSystemBodies(StarSystem *system, bool onRedo = false) :
			m_system(system),
			m_onRedo(onRedo)
		{
			Redo();
		}

		void Undo() override {
			if (!m_onRedo)
				StarSystem::EditorAPI::ReorderBodyIndex(m_system);
		}

		void Redo() override {
			if (m_onRedo)
				StarSystem::EditorAPI::ReorderBodyIndex(m_system);
		}

	private:
		StarSystem *m_system;
		bool m_onRedo;
	};

	// Helper to sort body hierarchy from index at the end of an undo / redo operation by adding two undo steps
	class SortStarSystemBodies : public UndoStep {
	public:
		SortStarSystemBodies(StarSystem *system, bool isRedo) :
			m_system(system),
			m_isRedo(isRedo)
		{
		}

		void Undo() override {
			if (!m_isRedo)
				StarSystem::EditorAPI::ReorderBodyHierarchy(m_system);
		}

		void Redo() override {
			if (m_isRedo)
				StarSystem::EditorAPI::ReorderBodyHierarchy(m_system);
		}

	private:
		StarSystem *m_system;
		bool m_isRedo;
	};

	// UndoStep helper to handle adding or deleting a child SystemBody from a parent
	class AddRemoveChildBody : public UndoStep {
	public:
		AddRemoveChildBody(SystemBody *parent, SystemBody *add, size_t idx) :
			m_parent(parent),
			m_add(add),
			m_idx(idx)
		{
			Swap();
		}

		AddRemoveChildBody(SystemBody *parent, SystemBody *add) :
			m_parent(parent),
			m_add(add),
			m_idx(-1)
		{
			Swap();
		}

		AddRemoveChildBody(SystemBody *parent, size_t idx) :
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
