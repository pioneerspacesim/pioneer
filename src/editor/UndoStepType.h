// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "editor/UndoSystem.h"

namespace Editor {

	// ========================================================================
	//  UndoClosure Helper
	// ========================================================================

	// UndoClosure wraps an UndoStep with a closure to be executed after an
	// Undo() or Redo() operation.

	template<typename ClosureType, typename T>
	class UndoClosure : public T {
	public:
		template<typename ...Args>
		UndoClosure(ClosureType &&closure, Args ...args) :
			T(args...),
			m_onUpdate(closure)
		{}

		void Swap() override {
			T::Swap();
			m_onUpdate();
		}

	private:
		ClosureType m_onUpdate;
	};

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
		}

		void Swap() override { std::swap(*m_dataRef, m_state); }

		// Implement HasChanged as !(a == b) to reduce the number of operator overloads required
		bool HasChanged() const override { return !(*m_dataRef == m_state); }

	private:
		ValueType *m_dataRef;
		ValueType m_state;
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
		s->AddUndoStep<UndoSingleValueStep<T>>(dataRef, newValue)->Swap();
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoSingleValueClosure(UndoSystem *s, T *dataRef, UpdateClosure closure)
	{
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoSingleValueStep<T>>>(std::move(closure), dataRef);
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoSingleValueClosure(UndoSystem *s, T *dataRef, const T &newValue, UpdateClosure closure)
	{
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoSingleValueStep<T>>>(std::move(closure), dataRef, newValue)->Swap();
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

		// Convenience constructor to allow updating the value in a single call
		UndoGetSetValueStep(Obj *data, const ValueType &newValue) :
			m_dataRef(data),
			m_state(newValue)
		{
		}

		// two-way swap with opaque setter/getter functions
		void Swap() override {
			ValueType t = (m_dataRef->*GetterFn)();
			std::swap(t, m_state);
			(m_dataRef->*SetterFn)(std::move(t));
		}

		bool HasChanged() const override { return !((m_dataRef->*GetterFn)() == m_state); }

	private:
		Obj *m_dataRef;
		ValueType m_state;
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
		s->AddUndoStep<UndoGetSetValueStep<GetterFn, SetterFn, ValueType, Obj>>(dataRef, newValue)->Swap();
	}

	template<auto GetterFn, auto SetterFn, typename Obj, typename UpdateClosure>
	inline void AddUndoGetSetValueClosure(UndoSystem *s, Obj *dataRef, UpdateClosure &&closure)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoGetSetValueStep<GetterFn, SetterFn, ValueType, Obj>>>(std::move(closure), dataRef);
	}

	template<auto GetterFn, auto SetterFn, typename Obj, typename T, typename UpdateClosure>
	inline void AddUndoGetSetValueClosure(UndoSystem *s, Obj *dataRef, const T &newValue, UpdateClosure &&closure)
	{
		using ValueType = decltype((dataRef->*GetterFn)());
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoGetSetValueStep<GetterFn, SetterFn, ValueType, Obj>>>(std::move(closure), dataRef, newValue)->Swap();
	}

	// ========================================================================
	//  UndoVectorStep Helper
	// ========================================================================

	// UndoVectorStep implements an UndoStep that expresses mutation of a
	// std::vector or similar container variable providing insert(), erase(),
	// begin(), and size().
	//
	// The only requirements are that the value type be Move-Constructible or
	// Copy-Constructible, and that the container's memory location will not
	// change between creation of the UndoStep and when it is undone/redone.

	template<typename Container>
	class UndoVectorStep : public UndoStep
	{
	public:
		using ValueType = typename Container::value_type;

		UndoVectorStep(Container *container, const ValueType &newValue, size_t insertIdx) :
			m_container(*container),
			m_value {},
			m_idx(insertIdx),
			m_insert(true)
		{
			// NOTE: the transaction is not yet committed until Swap() is called (for compatibility with UndoClosure)
		}

		UndoVectorStep(Container *container, size_t removeIdx) :
			m_container(*container),
			m_value {},
			m_idx(removeIdx),
			m_insert(false)
		{
			// NOTE: the transaction is not yet committed until Swap() is called (for compatibility with UndoClosure)
		}

		void Swap() override
		{
			if (m_insert) {
				m_container.insert(m_container.begin() + m_idx, std::move(m_value));
			} else {
				m_value = std::move(m_container[m_idx]);
				m_container.erase(m_container.begin() + m_idx);
			}

			m_insert = !m_insert;
		}

	private:
		Container &m_container;
		ValueType m_value;
		size_t m_idx;
		bool m_insert;
	};

	// ========================================================================
	//  UndoVectorSingleValue Helper
	// ========================================================================

	// UndoVectorSingleValue implements an UndoStep that expresses mutation of
	// a value contained in a std::vector or similar container variable
	// providing operator[](size_t) and size()
	//
	// The only requirements are that the value type be Move-Constructible or
	// Copy-Constructible, and that the container's memory location will not
	// change between creation of the UndoStep and when it is undone/redone.
	//
	// For containers that reallocate memory on mutation (e.g. std::vector)
	// this UndoStep is a replacement for UndoSingleValueStep.

	template<typename Container>
	class UndoVectorSingleValueStep : public UndoStep
	{
	public:
		using ValueType = typename Container::value_type;

		UndoVectorSingleValueStep(Container *container, size_t idx) :
			m_container(*container),
			m_index(idx),
			m_state(m_container[idx])
		{
		}

		void Swap() override { std::swap(m_container[m_index], m_state); }

		bool HasChanged() const override { return !(m_container[m_index] == m_state); }

	private:
		Container &m_container;
		size_t m_index;
		ValueType m_state;
	};

	// Helper functions to construct the above UndoStep helpers

	template<typename T, typename Value = typename T::value_type>
	inline void AddUndoVectorInsert(UndoSystem *s, T *containerRef, const Value &value, size_t idx = -1)
	{
		if (idx == size_t(-1))
			idx = containerRef->size();

		s->AddUndoStep<UndoVectorStep<T>>(containerRef, value, idx)->Swap();
	}

	template<typename T>
	inline void AddUndoVectorErase(UndoSystem *s, T *containerRef, size_t idx = -1)
	{
		if (idx == size_t(-1))
			idx = containerRef->size() - 1;

		s->AddUndoStep<UndoVectorStep<T>>(containerRef, idx)->Swap();
	}

	template<typename T>
	inline void AddUndoVectorSingleValue(UndoSystem *s, T *containerRef, size_t idx = -1)
	{
		if (idx == size_t(-1))
			idx = containerRef->size() - 1;

		s->AddUndoStep<UndoVectorSingleValueStep<T>>(containerRef, idx);
	}

	template<typename T, typename UpdateClosure, typename Value = typename T::value_type>
	inline void AddUndoVectorInsertClosure(UndoSystem *s, T *containerRef, const Value &value, size_t idx, UpdateClosure closure)
	{
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoVectorStep<T>>>(std::move(closure), containerRef, value, idx)->Swap();
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoVectorEraseClosure(UndoSystem *s, T *containerRef, size_t idx, UpdateClosure closure)
	{
		s->AddUndoStep<UndoClosure<UpdateClosure, UndoVectorStep<T>>>(std::move(closure), containerRef, idx)->Swap();
	}

	template<typename T, typename UpdateClosure>
	inline void AddUndoVectorSingleValue(UndoSystem *s, T *containerRef, size_t idx, UpdateClosure closure)
	{
		if (idx == size_t(-1))
			idx = containerRef->size() - 1;

		s->AddUndoStep<UndoClosure<UpdateClosure, UndoVectorSingleValueStep<T>>>(std::move(closure), containerRef, idx);
	}

} // namespace Editor
