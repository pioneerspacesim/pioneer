// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Application.h"
#include "FileSystem.h"
#include "OS.h"
#include "SDL.h"
#include "profiler/Profiler.h"
#include "utils.h"

#include "SDL_timer.h"

#include <stdexcept>

void Application::QueueLifecycle(std::shared_ptr<Lifecycle> cycle)
{
	if (!cycle)
		throw std::runtime_error("Invalid Lifecycle object pushed to Application queue!");

	m_queuedLifecycles.push(cycle);
}

void Application::Startup()
{
	PROFILE_SCOPED()

	OS::EnableBreakpad();
	OS::NotifyLoadBegin();

	FileSystem::Init();
	// ensure the config directory exists
	FileSystem::userFiles.MakeDirectory("");

#ifdef PIONEER_PROFILER
	FileSystem::userFiles.MakeDirectory("profiler");
#endif

	SDL_Init(SDL_INIT_EVENTS);
}

void Application::Shutdown()
{
	FileSystem::Uninit();
	SDL_Quit();
}

bool Application::StartLifecycle()
{
	// can't start a lifecycle if there are no more queued.
	if (!m_priorityLifecycle && !m_queuedLifecycles.size())
		return false;

	// If we still have an active lifestyle, we don't need to queue any more.
	if (m_activeLifecycle)
		throw std::runtime_error("Attempt to start a new lifecycle object while one is still active!");

	// Lifecycle objects returned from Lifecycle::End() take priority over queued objects
	if (m_priorityLifecycle) {
		m_activeLifecycle = m_priorityLifecycle;
		m_priorityLifecycle = nullptr;
	} else {
		m_activeLifecycle = std::move(m_queuedLifecycles.front());
		m_queuedLifecycles.pop();
	}

	m_activeLifecycle->m_application = this;
	m_activeLifecycle->Start();
	return true;
}

void Application::EndLifecycle()
{
	// Can't end a lifecycle if we don't have one active.
	if (!m_activeLifecycle)
		return;

	// The only time we should have a prioritized lifecycle is
	// between EndLifecycle() and the next StartLifecycle().
	// If it's still set, something has gone wrong.
	if (m_priorityLifecycle)
		throw std::runtime_error("Unable to prioritize lifecycle object due to already-prioritized lifecycle. Did you mess up your control flow?");

	m_activeLifecycle->End();
	m_activeLifecycle->m_application = nullptr;
	m_activeLifecycle->m_endLifecycle = false;

	// wait until we've finished the control flow for the lifecycle;
	// the lifecycle may decide to set the next lifecycle in End()
	m_priorityLifecycle = m_activeLifecycle->m_nextLifecycle;
	m_activeLifecycle = nullptr;
}

void Application::ClearQueuedLifecycles()
{
	while (m_queuedLifecycles.size())
		m_queuedLifecycles.pop();
}

void Application::Run()
{
	Profiler::Clock m_runtime{};
	m_runtime.Start();
	m_applicationRunning = true;

	Startup();

	if (!m_queuedLifecycles.size())
		throw std::runtime_error("Application::Run must have a queued lifecycle object (did you forget to queue one?)");

#ifdef PIONEER_PROFILER
	// For good measure, reset the profiler at the start of the first frame
	Profiler::reset();
#endif

	// SoftStop updates the elapsed time measured by the clock, and continues to run the clock.
	m_runtime.SoftStop();
	m_totalTime = m_runtime.seconds();
	while (m_applicationRunning) {
		m_runtime.SoftStop();
		double thisTime = m_runtime.seconds();
		m_deltaTime = thisTime - m_totalTime;
		m_totalTime = thisTime;

		if (!m_activeLifecycle) {
			if (!StartLifecycle())
				break;
		}

		// If there is no lifecycle object to start, end now.
		if (!m_activeLifecycle)
			break;

		BeginFrame();

		// The PreUpdate hook should be used for setting up per-frame state, etc.
		PreUpdate();

		m_activeLifecycle->Update(m_deltaTime);

		// The PostUpdate hook should be used for finalizing per-frame state, rendering, etc.
		PostUpdate();

		EndFrame();

		if (m_activeLifecycle->m_endLifecycle || !m_applicationRunning) {
			EndLifecycle();
		}

		// TODO: design a better profiling interface, cache results of slow frames so they can be inspected
		// in pigui, etc.
		// m_runtime.SoftStop();
		// thisTime = m_runtime.seconds();
		// if (thisTime - m_totalTime > 0.100) // profile frames taking longer than 100ms
		// 	Profiler::dumphtml(FileSystem::JoinPathBelow(FileSystem::GetUserDir(), "profiler");

#ifdef PIONEER_PROFILER
		// reset the profiler at the end of the frame
		if (!m_activeLifecycle || !m_activeLifecycle->m_profilerAccumulate)
			Profiler::reset();
#endif
	}

	Shutdown();
}
