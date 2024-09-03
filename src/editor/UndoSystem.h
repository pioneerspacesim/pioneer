// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "core/StringName.h"

#include <memory>
#include <string_view>
#include <vector>

namespace Editor {

/*
 * UndoStep represents a single unit of work (a single "change") made as part
 * of some user action.
 *
 * An UndoStep should store only enough state to be able to undo or redo said
 * change, as the non-transient state of the application is expected to be
 * exactly the same no matter how many times the user has undone/redone the
 * step or any other steps around it.
 *
 * Changes to transient application data (e.g. caches, derived data, etc)
 * should be triggered immediately by the UndoStep where possible to maintain
 * consistency in all areas of the application's data model.
 *
 * It is invalid for the Undo() or Redo() methods to directly or indirectly
 * create a new UndoEntry or UndoStep while being executed, as this will
 * corrupt the undo manager's state.
 */
class UndoStep {
public:
	UndoStep() = default;
	virtual ~UndoStep() = default;

	// Execute an undo step (replace application state with stored state)
	virtual void Undo() { Swap(); }

	// Execute a redo step (replace stored state with application state)
	virtual void Redo() { Swap(); }

	// Helper method to allow state changes in a single function
	virtual void Swap() {};

	// Optimization step: entries for which none of the steps represent a
	// change in state will not be added to the undo stack
	virtual bool HasChanged() const { return true; }
};

/*
 * An UndoEntry is a mostly implementation-specific detail - it represents a
 * high-level action by the user (e.g. Transform Object) at the most granular
 * level of undo. One Ctrl+Z should undo one UndoEntry, and vice versa.
 *
 * All managed UndoSteps in an entry are applied in program order to maintain
 * application state consistency (back-to-front undo, front-to-back redo).
 */
class UndoEntry {
public:
	std::string_view GetName() const { return m_name.sv(); }

private:
	friend class UndoSystem;
	UndoEntry() = default;

	void Undo();
	void Redo();

	bool HasChanged() const;

	StringName m_name;
	size_t m_id;
	std::vector<std::unique_ptr<UndoStep>> m_steps;
};

/*
 * The UndoSystem provides a central place to manage undo entries for a
 * specific "undo queue" (e.g. document, window, editor, etc).
 *
 * It maintains separate undo/redo stacks under the hood, and allows for
 * recursion of entries to allow maximum flexibility in calling code.
 *
 * Recursive BeginEntry() calls must be matched with EndEntry() calls, and the
 * name of sub-entries is lost. Undo operates on the granularity of top-level
 * UndoEntries, so sub-entries only allow composing program operations that
 * would otherwise require multiple undo interactions for a single user action.
 *
 * AddUndoStep() creates a new undo step and adds it to the currently active
 * UndoEntry. The Redo method of the undo entry is not triggered automatically,
 * as the semantics of an UndoStep are simpler without the Undo/Do/Redo split.
 *
 * Calling BeginEntry() / ResetEntry() / EndEntry() / AddUndoStep() while an
 * UndoEntry is being undone or redone is a violation of program state and will
 * cause a crash.
 *
 * ============================================================================
 *
 * A common pattern is to begin an UndoEntry when the user signals they want
 * to begin editing (e.g. user clicks on a transform handle) and update that
 * entry while the user performs some continuous action (such as dragging).
 *
 * There are two options to achieve this pattern: if your UndoSteps store only
 * the "prior state" of the application, you can call BeginEntry() once, submit
 * one or more UndoSteps, and modify application state as needed over multiple
 * frames before calling EndEntry().
 *
 * Alternatively, if your undo steps must know both the prior and current state
 * of the application, you can call ResetEntry() each frame to undo and discard
 * all submitted UndoSteps() in the currently active entry. All submitted steps
 * will be undone, regardless of BeginEntry() recursion level.
 */
class UndoSystem {
public:
	UndoSystem();
	~UndoSystem();

	// Return the total number of undo/redo entries in the stack
	size_t GetNumEntries() const;

	// Return the index of the "current entry" that describes application state.
	// This points to the first entry in the redo stack, if any.
	size_t GetCurrentEntry() const;

	// Return a pointer to the given undo entry if stackIdx < GetNumEntries().
	const UndoEntry *GetEntry(size_t stackIdx) const;

	// Calculate a hash value used to describe the current undo state
	size_t GetStateHash();

	bool CanUndo() const { return !m_undoStack.empty(); }
	bool CanRedo() const { return !m_redoStack.empty(); }

	// Undo the most recent entry in the undo stack (rewind the application state)
	void Undo();

	// Redo the most recent entry in the redo stack (advance the application state)
	void Redo();

	// Destroy all undo/redo history (e.g. when closing a file)
	void Clear();

	// Begin a new undo entry (can be recursive)
	void BeginEntry(std::string_view name);

	// Return the number of open entries (for debugging purposes)
	size_t GetEntryDepth() const { return m_openUndoDepth; }

	// Reset the current undo entry to a "clean state"
	void ResetEntry();

	// Add an undo step to the active undo entry
	template<typename T, typename ...Args>
	T *AddUndoStep(Args&& ...args)
	{
		T *step = new T(std::forward<Args>(args)...);
		AddUndoStep(step);
		return step;
	}

	// End the current undo entry
	void EndEntry();

private:
	void AddUndoStep(UndoStep *undo);

	std::vector<std::unique_ptr<UndoEntry>> m_undoStack;
	std::vector<std::unique_ptr<UndoEntry>> m_redoStack;

	std::unique_ptr<UndoEntry> m_openUndoEntry;
	size_t m_openUndoDepth;
	size_t m_entryId;

	bool m_doing;
};

} // namespace Editor
