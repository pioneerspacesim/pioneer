// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "editor/UndoSystem.h"

namespace Editor {

	// ========================================================================
	//  UndoSingleValue Helper
	// ========================================================================

	// UndoSingleValueStep implements an UndoStep that allows tracking mutation
	// of a single public member variable in any class.
	//
	// The only requirements are that the value type be Move-Constructible or
	// Copy-Constructible, and that the value's memory location will not change
	// between creation of the UndoStep and when it is undone/redone.

	template<typename ValueType>
	class UndoSingleValueStep : public UndoStep
	{
	public:
		// Simple copy-state constructor to clone the given value
		UndoSingleValueStep(ValueType *data) :
			m_dataRef(data),
			m_state(*m_dataRef)
		{
		}

		// Convenience constructor overload to allow updating a data value in a single call
		// (allows UndoSingleValueStep to work for move-only values like std::unique_ptr)
		UndoSingleValueStep(ValueType *data, const ValueType &newValue) :
			m_dataRef(data),
			m_state(newValue)
		{
			std::swap(m_state, *m_dataRef);
		}

		void Undo() override { std::swap(*m_dataRef, m_state); }
		void Redo() override { std::swap(*m_dataRef, m_state); }

		// Implement HasChanged as !(a == b) to reduce the number of operator overloads required
		bool HasChanged() const override { return !(*m_dataRef == m_state); }

	private:
		ValueType *m_dataRef;
		ValueType m_state;
	};

	template<typename ValueType, typename ClosureType>
	class UndoSingleValueClosureStep : public UndoStep
	{
	public:
		UndoSingleValueClosureStep(ValueType *type, ClosureType &&updateClosure) :
			m_dataRef(type),
			m_state(*m_dataRef),
			m_onUpdate(std::move(updateClosure))
		{
		}

		UndoSingleValueClosureStep(ValueType *type, const ValueType &newValue, ClosureType &&updateClosure) :
			m_dataRef(type),
			m_state(newValue),
			m_onUpdate(std::move(updateClosure))
		{
			std::swap(*m_dataRef, m_state);
			m_onUpdate();
		}

		void Undo() override
		{
			std::swap(*m_dataRef, m_state);
			m_onUpdate();
		}

		void Redo() override
		{
			std::swap(*m_dataRef, m_state);
			m_onUpdate();
		}

		bool HasChanged() const override { return !(*m_dataRef == m_state); }

	private:
		ValueType *m_dataRef;
		ValueType m_state;

		ClosureType m_onUpdate;
	};

	// Helper functions to construct the above UndoStep helpers

	template<typename T>
	inline void AddUndoSingleValue(UndoSystem *s, T *dataRef)
	{
		s->AddUndoStep<UndoSingleValueStep<T>>(dataRef);
	}

	template<typename T>
	inline void AddUndoSingleValue(UndoSystem *s, T *dataRef, const T &newValue)
	{
		s->AddUndoStep<UndoSingleValueStep<T>>(dataRef, newValue);
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoSingleValueClosure(UndoSystem *s, T *dataRef, UpdateClosure &&closure)
	{
		s->AddUndoStep<UndoSingleValueClosureStep<T, UpdateClosure>>(dataRef, std::move(closure));
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoSingleValueClosure(UndoSystem *s, T *dataRef, const T &newValue, UpdateClosure &&closure)
	{
		s->AddUndoStep<UndoSingleValueClosureStep<T, UpdateClosure>>(dataRef, newValue, std::move(closure));
	}

	// ========================================================================
	//  UndoGetSetValue Helper
	// ========================================================================

	// UndoGetSetValueStep implements an UndoStep that allows tracking mutation
	// of a single private member variable exposed by a GetX() / SetX() pair.
	//
	// The value type must be Copy-Constructible, and the object containing the
	// variable must not be reallocated or destroyed during the lifetime of the
	// UndoStep.

	template<auto GetterFn, auto SetterFn, typename ValueType, typename Obj>
	class UndoGetSetValueStep : public UndoStep
	{
	public:
		UndoGetSetValueStep(Obj *data) :
			m_dataRef(data),
			m_state((m_dataRef->*GetterFn)())
		{
		}

		UndoGetSetValueStep(Obj *data, const ValueType &newValue) :
			m_dataRef(data),
			m_state(newValue)
		{
			swap();
		}

		void Undo() override { swap(); }
		void Redo() override { swap(); }

		bool HasChanged() const override { return !((m_dataRef->*GetterFn)() == m_state); }

	private:
		// two-way swap with opaque setter/getter functions
		void swap() {
			ValueType t = (m_dataRef->*GetterFn)();
			std::swap(t, m_state);
			(m_dataRef->*SetterFn)(std::move(t));
		}

		Obj *m_dataRef;
		ValueType m_state;
	};

	template<auto GetterFn, auto SetterFn, typename ValueType, typename Obj, typename ClosureType>
	class UndoGetSetValueClosureStep : public UndoStep
	{
	public:
		UndoGetSetValueClosureStep(Obj *data, ClosureType &&updateClosure) :
			m_dataRef(data),
			m_state((m_dataRef->*GetterFn)()),
			m_update(std::move(updateClosure))
		{
		}

		UndoGetSetValueClosureStep(Obj *data, const ValueType &newValue, ClosureType &&updateClosure) :
			m_dataRef(data),
			m_state(newValue),
			m_update(std::move(updateClosure))
		{
			swap();
		}

		void Undo() override { swap(); }
		void Redo() override { swap(); }

		bool HasChanged() const override { return !((m_dataRef->*GetterFn)() == m_state); }

	private:
		// two-way swap with opaque setter/getter functions and update closure
		void swap() {
			ValueType t = (m_dataRef->*GetterFn)();
			std::swap(t, m_state);
			(m_dataRef->*SetterFn)(std::move(t));

			m_update();
		}

		Obj *m_dataRef;
		ValueType m_state;
		ClosureType m_update;
	};

	// Helper functions to construct the above UndoStep helpers

	template<auto GetterFn, auto SetterFn, typename Obj>
	inline void AddUndoGetSetValue(UndoSystem *s, Obj *dataRef)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoGetSetValueStep<GetterFn, SetterFn, ValueType, Obj>>(dataRef);
	}

	template<auto GetterFn, auto SetterFn, typename Obj, typename T>
	inline void AddUndoGetSetValue(UndoSystem *s, Obj *dataRef, const T &newValue)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoGetSetValueStep<GetterFn, SetterFn, ValueType, Obj>>(dataRef, newValue);
	}

	template<auto GetterFn, auto SetterFn, typename Obj, typename UpdateClosure>
	inline void AddUndoGetSetValueClosure(UndoSystem *s, Obj *dataRef, UpdateClosure &&closure)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoGetSetValueClosureStep<GetterFn, SetterFn, ValueType, Obj, UpdateClosure>>(dataRef, std::move(closure));
	}

	template<auto GetterFn, auto SetterFn, typename Obj, typename T, typename UpdateClosure>
	inline void AddUndoGetSetValueClosure(UndoSystem *s, Obj *dataRef, const T &newValue, UpdateClosure &&closure)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoGetSetValueClosureStep<GetterFn, SetterFn, ValueType, Obj, UpdateClosure>>(dataRef, newValue, std::move(closure));
	}

} // namespace Editor
