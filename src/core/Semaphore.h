//---------------------------------------------------------
// Heavily adapted version of:
// sema.h - C++11 On Multicore
// Copyright (c) 2015 Jeff Preshing
// Released under the zlib License
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#pragma once

#include "SDL_mutex.h"
#include <atomic_queue/defs.h>

// Lightweight spinlock semaphore
class Semaphore {
public:
	Semaphore(uint32_t initialCount = 0) :
		m_sem(SDL_CreateSemaphore(0)),
		m_count(initialCount)
	{}

	~Semaphore()
	{
		SDL_DestroySemaphore(m_sem);
	}

	// return the number of waiting threads
	int32_t count() const { return -m_count.load(std::memory_order_acquire); }

	bool try_wait()
	{
		int value = m_count.load(std::memory_order_relaxed);
		return (value > 0 && m_count.compare_exchange_strong(value, value - 1, std::memory_order_acquire));
	}

	void wait()
	{
		if (!try_wait())
			wait_with_spinlock();
	}

	// immediately wait without spinning if the semaphore is not signalled
	void waitonly()
	{
		if (!try_wait()) {
			int value = m_count.fetch_sub(1, std::memory_order_acquire);
			if (value <= 0)
				SDL_SemWait(m_sem);
		}
	}

	void signal(int count = 1)
	{
		int value = m_count.fetch_add(count, std::memory_order_release);
		int toRelease = -value < count ? -value : count;
		while (toRelease-- > 0) {
			SDL_SemPost(m_sem);
		}
	}

private:
	void wait_with_spinlock()
	{
		uint32_t spinCount = 1000;
		int32_t value = 0;

		while (spinCount--) {
			value = m_count.load(std::memory_order_relaxed);
			if ((value > 0) && m_count.compare_exchange_strong(value, value - 1, std::memory_order_acquire))
				return;
			atomic_queue::spin_loop_pause();
			// keep the compiler from detrimentally optimizing the loop
			std::atomic_signal_fence(std::memory_order_acquire);
		}

		value = m_count.fetch_sub(1, std::memory_order_acquire);
		if (value <= 0)
			SDL_SemWait(m_sem);
	}

	SDL_semaphore *m_sem;
	std::atomic<int32_t> m_count;
};
