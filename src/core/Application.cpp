// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Application.h"
#include "FileSystem.h"
#include "JobQueue.h"
#include "OS.h"
#include "SDL.h"
#include "StringName.h"
#include "TaskGraph.h"
#include "profiler/Profiler.h"

#include "SDL_timer.h"

#include <stdexcept>

static constexpr Uint32 SYNC_JOBS_PER_LOOP = 1;

Application::Application() {}
Application::~Application() {}

void Application::QueueLifecycle(RefCountedPtr<Lifecycle> cycle)
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

	m_taskGraph = std::make_unique<TaskGraph>();
	m_syncJobQueue = std::make_unique<SyncJobQueue>();

	FileSystem::Init();
	// ensure the config directory exists
	FileSystem::userFiles.MakeDirectory("");

#ifdef PIONEER_PROFILER
	FileSystem::userFiles.MakeDirectory("profiler");
#endif

	SDL_Init(SDL_INIT_EVENTS);

	OnStartup();
}

void Application::Shutdown()
{
	OnShutdown();

	m_taskGraph.reset();
	m_syncJobQueue.reset();

	FileSystem::Uninit();
	SDL_Quit();
}

void Application::RequestProfileFrame(const std::string &path)
{
	// don't do anything if we're building without profiler.
#ifdef PIONEER_PROFILER
	if (!path.empty()) {
		m_tempProfilePath = FileSystem::JoinPathBelow(m_profilerPath, path);
		FileSystem::userFiles.MakeDirectory(m_tempProfilePath);
	}

	m_doTempProfile = true;
#endif
}

void Application::SetProfilerPath(const std::string &path)
{
#ifdef PIONEER_PROFILER
	if (path.empty())
		return;

	m_profilerPath = path;
	FileSystem::userFiles.MakeDirectory(m_profilerPath);
#endif
}

JobQueue *Application::GetSyncJobQueue()
{
	return m_syncJobQueue.get();
}

JobQueue *Application::GetAsyncJobQueue()
{
	return m_taskGraph->GetJobQueue();
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
		m_priorityLifecycle.Reset();
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
	m_activeLifecycle.Reset();
}

void Application::ClearQueuedLifecycles()
{
	while (m_queuedLifecycles.size())
		m_queuedLifecycles.pop();
}

void Application::HandleJobs()
{
	m_taskGraph->RunPinnedTasks();
	m_syncJobQueue->RunJobs(SYNC_JOBS_PER_LOOP);
	m_syncJobQueue->FinishJobs();
	m_taskGraph->GetJobQueue()->FinishJobs();

	// Reclaim StringTable memory periodically
	StringTable::Get()->Reclaim();
}

void Application::Run()
{
	if (!m_queuedLifecycles.size())
		throw std::runtime_error("Application::Run must have a queued lifecycle object (did you forget to queue one?)");

	// SoftStop updates the elapsed time measured by the clock, and continues to run the clock.
	Profiler::Clock m_runtime{};
	m_runtime.Start();
	m_runtime.SoftStop();
	m_totalTime = m_runtime.seconds();

	m_applicationRunning = true;
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

		HandleJobs();

		// The PostUpdate hook should be used for finalizing per-frame state, rendering, etc.
		PostUpdate();

		EndFrame();

#ifdef PIONEER_PROFILER
		const bool profileReset = (m_activeLifecycle && !m_activeLifecycle->m_profilerAccumulate);
#endif

		if (m_activeLifecycle->m_endLifecycle || !m_applicationRunning) {
			EndLifecycle();
		}

#ifdef PIONEER_PROFILER
		// TODO: potential pigui frame profile inspector
		m_runtime.SoftStop();
		thisTime = m_runtime.seconds();

		// profile frames taking longer than 100ms
		bool isSlowProfile = (m_doSlowProfile && thisTime - m_totalTime > 0.100);
		if (m_doTempProfile || isSlowProfile) {
			const std::string path = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(),
				m_tempProfilePath.empty() ? m_profilerPath : m_tempProfilePath);
			m_tempProfilePath.clear();
			m_doTempProfile = false;

			Profiler::dumphtml(path.c_str());
			if (m_profileZones) {
				if (m_profileTrace)
					Profiler::dumptrace(path.c_str());
				else
					Profiler::dumpzones(path.c_str());
			}
		}

		// reset the profiler at the end of the frame
		if (profileReset)
			Profiler::reset();
#endif
	}
}
