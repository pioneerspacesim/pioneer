// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UndoSystem.h"
#include "utils.h"

#define XXH_INLINE_ALL
#include "lz4/xxhash.h"

#include <cassert>

using namespace Editor;

void UndoEntry::Undo()
{
	for (auto &step : reverse_container(m_steps))
		step->Undo();
}

void UndoEntry::Redo()
{
	for (auto &step : m_steps)
		step->Redo();
}

bool UndoEntry::HasChanged() const
{
	for (auto &step : m_steps)
		if (step->HasChanged())
			return true;

	return false;
}

// ============================================================================

UndoSystem::UndoSystem() :
	m_openUndoDepth(0),
	m_entryId(0),
	m_doing(false)
{
}

UndoSystem::~UndoSystem()
{
}

size_t UndoSystem::GetNumEntries() const
{
	return m_undoStack.size() + m_redoStack.size();
}

size_t UndoSystem::GetCurrentEntry() const
{
	return m_undoStack.size();
}

const UndoEntry *UndoSystem::GetEntry(size_t stackIdx) const
{
	if (stackIdx < m_undoStack.size())
		return m_undoStack[stackIdx].get();

	stackIdx -= m_undoStack.size();

	if (stackIdx < m_redoStack.size()) // redo is a reverse stack
		return m_redoStack.rbegin()[stackIdx].get();

	return nullptr;
}

size_t UndoSystem::GetStateHash()
{
	size_t hash = "UndoState"_hash;
	for (auto &entry_ptr : m_undoStack)
		hash = XXH64(&entry_ptr->m_id, sizeof(size_t), hash);

	return hash;
}

void UndoSystem::Undo()
{
	if (!CanUndo())
		return;

	UndoEntry *entry = m_undoStack.back().release();
	m_undoStack.pop_back();

	m_doing = true;
	entry->Undo();
	m_doing = false;

	m_redoStack.emplace_back(entry);
}

void UndoSystem::Redo()
{
	if (!CanRedo())
		return;

	UndoEntry *entry = m_redoStack.back().release();
	m_redoStack.pop_back();

	m_doing = true;
	entry->Redo();
	m_doing = false;

	m_undoStack.emplace_back(entry);
}

void UndoSystem::Clear()
{
	// It's probably a bad idea to call Clear() while redoing an UndoEntry
	assert(!m_doing && "Cannot clear UndoSystem state while inside an undo step!");

	m_openUndoEntry.reset();
	m_openUndoDepth = 0;

	m_entryId = 0;

	m_redoStack.clear();
	m_undoStack.clear();
}

void UndoSystem::BeginEntry(std::string_view name)
{
	assert(!m_doing && "Cannot begin an entry inside an undo step!");

	// Recursive entries are mostly supported to allow chaining operations that individually would also push undo entries
	if (m_openUndoDepth++ > 0)
		return;

	m_openUndoEntry.reset(new UndoEntry());
	m_openUndoEntry->m_name = name;
	m_openUndoEntry->m_id = ++m_entryId;
}

void UndoSystem::ResetEntry()
{
	assert(!m_doing && "Cannot reset an entry inside an undo step!");
	assert(m_openUndoDepth > 0 && "Cannot reset an undo entry without having begun one prior");

	m_openUndoEntry->Undo();
	m_openUndoEntry->m_steps.clear();
}

void UndoSystem::EndEntry()
{
	assert(!m_doing && "Cannot end an entry inside an undo step!");

	// Recursive entries don't need to do any cleanup
	if (--m_openUndoDepth > 0)
		return;

	// if the entry represents a change to the application state, commit it to
	// the undo stack and clear redo state
	if (m_openUndoEntry->HasChanged()) {
		UndoEntry *entry = m_openUndoEntry.release();
		m_undoStack.emplace_back(entry);

		if (!m_redoStack.empty())
			m_redoStack.clear();
	} else {
		// otherwise, just get rid of the entry without touching redo state
		m_openUndoEntry.reset();
	}
}

void UndoSystem::AddUndoStep(UndoStep *undo)
{
	assert(!m_doing && "Cannot add an undo step inside an undo step!");
	assert(m_openUndoEntry && "Cannot add an undo step without a valid open undo entry!");

	m_openUndoEntry->m_steps.emplace_back(undo);
}
