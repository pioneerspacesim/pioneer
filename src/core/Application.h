// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"

#include <memory>
#include <queue>
#include <string>

class JobQueue;
class SyncJobQueue;
class TaskGraph;

class Application {
public:
	Application();
	virtual ~Application();

	class Lifecycle : public RefCounted {
	public:
		Lifecycle(){};
		Lifecycle(bool profilerAccumulate) :
			m_profilerAccumulate(profilerAccumulate){};
		virtual ~Lifecycle(){};

		// Once called, the lifecycle is terminated at the end of the current update.
		void RequestEndLifecycle() { m_endLifecycle = true; }

	protected:
		friend class Application;

		// Called when the Lifecycle begins execution
		virtual void Start(){};

		// Called in a continual loop and passed the time since the last invocation.
		virtual void Update(float deltaTime) = 0;

		// Called when the lifecycle is leaving execution
		// If a valid Lifecycle pointer is returned, it is run before any queued lifecycles.
		virtual void End() {}

		// Set a lifecycle that should begin immediately after this lifecycle has finished execution
		void SetNextLifecycle(RefCountedPtr<Lifecycle> l)
		{
			m_nextLifecycle = l;
		}

		bool GetProfilerAccumulate() const { return m_profilerAccumulate; }

		void SetProfilerAccumulate(bool enable)
		{
			m_profilerAccumulate = enable;
		}

	private:
		// set to true when you want to accumulate all updates in a lifecycle into a single profile frame
		bool m_profilerAccumulate = false;
		bool m_endLifecycle = false;
		RefCountedPtr<Lifecycle> m_nextLifecycle;
		Application *m_application;
	};

	// Add a lifecycle object to the queue of pending lifecycles
	// You must add at least one lifecycle before calling Run()
	void QueueLifecycle(RefCountedPtr<Lifecycle> cycle);

	// Use this function very sparingly (e.g. when the application is shutting down)
	void ClearQueuedLifecycles();

	// Perform initialization tasks at program startup
	void Startup();

	// Runs the application as long as there is a valid lifecycle object
	void Run();

	// Perform deinitialization tasks at program termination
	void Shutdown();

	// Get the time between the start of the last update and the current one
	float DeltaTime() { return m_deltaTime; }

	double GetTime() { return m_totalTime; }

	TaskGraph *GetTaskGraph() { return m_taskGraph.get(); }

	JobQueue *GetSyncJobQueue();
	JobQueue *GetAsyncJobQueue();

	void RequestProfileFrame(const std::string &path = "");

protected:
	// Hooks for inheriting classes to add their own behaviors to.

	// Runs before the main loop begins
	virtual void OnStartup() {}

	// Runs after the main loop ends
	virtual void OnShutdown() {}

	// Handle running pinned tasks and processing queued jobs
	virtual void HandleJobs();

	// Runs at the top of each frame.
	virtual void BeginFrame() {}

	// Runs before each Update() call
	virtual void PreUpdate() {}

	// Runs after each Update() call
	virtual void PostUpdate() {}

	// Runs at the bottom of each frame.
	virtual void EndFrame() {}

	// Request the application quit immediately at the end of the update,
	// ignoring all queued lifecycles
	void RequestQuit() { m_applicationRunning = false; }

	Lifecycle *GetActiveLifecycle() { return m_activeLifecycle.Get(); }
	void SetProfilerPath(const std::string &);
	void SetProfileSlowFrames(bool enabled) { m_doSlowProfile = enabled; }
	void SetProfileZones(bool enabled) { m_profileZones = enabled; }
	void SetProfileTrace(bool enabled) { m_profileTrace = enabled; }

private:
	bool StartLifecycle();
	void EndLifecycle();

	bool m_applicationRunning = false;
	bool m_doTempProfile = false;
	bool m_doSlowProfile = false;
	bool m_profileZones = false;
	bool m_profileTrace = false;
	float m_deltaTime = 0.f;
	double m_totalTime = 0.f;

	std::string m_profilerPath;
	std::string m_tempProfilePath;

	// The lifecycle we're actually running right now
	RefCountedPtr<Lifecycle> m_activeLifecycle;

	// A lifecycle that should be run next before the rest of the queue.
	RefCountedPtr<Lifecycle> m_priorityLifecycle;

	// Lifecycles queued by QueueLifecycle()
	std::queue<RefCountedPtr<Lifecycle>> m_queuedLifecycles;

	std::unique_ptr<SyncJobQueue> m_syncJobQueue;
	std::unique_ptr<TaskGraph> m_taskGraph;
};
