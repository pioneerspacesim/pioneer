// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "core/Semaphore.h"

struct TaskRange {
	uint32_t begin;
	uint32_t end;
};

class AsyncTaskQueueImpl;
class AsyncJobQueueImpl;

class JobQueue;
class TaskGraphJobQueueImpl;
class TaskGraph;

// CompleteNotifier is a simple atomic helper class to track how many tasks
// are running vs complete
class CompleteNotifier {
public:
	CompleteNotifier() :
		m_dependants(0) {}

	std::atomic<uint32_t> m_dependants;

	bool IsComplete() { return m_dependants.load(std::memory_order_relaxed) == 0; }
};

// A Task is the building block of the TaskGraph system.
// It represents a single unit of work to be done in parallel fashion.
// Tasks are associated with a numeric range of elements to operate on;
// if you have a large number of items to process, create multiple tasks
// responsible for a subrange of the overall workload.
//
// Tasks are managed by raw pointers and should not be deleted by
// user code - the owning TaskSet or TaskGraph will delete the task
// when it is complete.
//
// Subclass and implement these methods:
//   OnExecute: responsible for carrying out the actual work done by the task,
//     runs on a worker thread.
//
//   OnComplete: used to synchronize reporting of task results, called by the
//     task owner at a synchronization point on the task owner's thread.
class Task {
public:
	Task(TaskRange range = {}) :
		m_owner(nullptr),
		m_range(range) {}
	virtual ~Task() = default;

	// Runs on a worker thread. Do all work in the task here.
	virtual void OnExecute(TaskRange range) = 0;

	// Run by the task owner on the owning thread when the task has finished.
	// If the task has no owner, this function will not be called.
	virtual void OnComplete(){};

	// Sets the task owner (responsible for calling task completion callbacks)
	void SetOwner(CompleteNotifier *);

private:
	friend class TaskGraph;
	CompleteNotifier *m_owner;
	TaskRange m_range;
};

// Helper task define to enable easy creation with lambdas.
template <typename Function>
class LambdaTask : public Task {
public:
	LambdaTask(TaskRange r, Function &&lambda) :
		Task(r),
		m_lambda(lambda) {}

	void OnExecute(TaskRange range) override { m_lambda(range); }
	void OnComplete() override {}

private:
	Function m_lambda;
};

// Represents a group of related tasks and is responsible for handling
// post-task operations on the main thread.
class TaskSet : public CompleteNotifier {
public:
	// A Handle is the runtime interface to a currently executing TaskSet.
	// If you do not call the (blocking) TaskGraph->WaitForTaskSet(handle);
	// you will need to check for the handle to be complete and manually
	// trigger execution of the task completion callbacks via
	// TaskGraph->CompleteTaskSet();
	struct Handle {
		Handle(const Handle &) = delete;
		Handle &operator=(const Handle &) = delete;
		Handle(Handle &&) = default;
		Handle &operator=(Handle &&) = default;
		bool IsComplete() { return !m_set || m_set->IsComplete(); }

	private:
		friend class TaskGraph;
		Handle(TaskSet *set) :
			m_set(set) {}

		TaskSet *m_set;
	};

	TaskSet() :
		m_tasks{},
		m_executing(false) {}

	// Add an individual task to this TaskSet.
	// This operation will fail if the task set is currently executing.
	bool AddTask(Task *task);

	// Adds a lambda task to this TaskSet.
	template <typename Function>
	bool AddTaskLambda(TaskRange range, Function &&fn)
	{
		return AddTask(new LambdaTask<Function>(range, std::move(fn)));
	}

	bool IsExecuting() { return m_executing; }

private:
	friend class TaskGraph;
	std::vector<Task *> m_tasks;
	bool m_executing;
};

// The global task orchestrator.
// Responsible for managing threads and executing tasks
class TaskGraph {
public:
	TaskGraph();
	~TaskGraph();

	// Set the total number of worker threads
	void SetWorkerThreads(uint32_t numThreads);
	uint32_t GetNumWorkerThreads() const;

	// Queues all tasks in a TaskSet for execution. The TaskGraph now owns
	// the underlying TaskSet and is responsible for deletion.
	TaskSet::Handle QueueTaskSet(TaskSet *set);
	// Queues a single task without a TaskSet for execution
	void QueueTask(Task *task);

	// Queues all tasks in a task set to be executed on the main thread only
	TaskSet::Handle QueueTaskSetPinned(TaskSet *set);
	// Queues a single task to be executed on the main thread only
	void QueueTaskPinned(Task *task);

	// Wait for a queued task set to complete and run task completion callbacks.
	// This will execute tasks on the calling thread until the given TaskSet
	// handle has completed all queued tasks.
	void WaitForTaskSet(TaskSet::Handle &set);

	// Runs completion callbacks for a TaskSet and destroys the underlying
	// TaskSet object when completed.
	// Returns false if the task set was not completed.
	bool CompleteTaskSet(TaskSet::Handle &set);

	// Run currently available tasks pinned to the main thread
	void RunPinnedTasks();

	// Return the JobQueue interface object for this task graph.
	JobQueue *GetJobQueue();

	static uint32_t GetThreadNum();

private:
	friend class TaskGraphJobQueueImpl;
	struct ThreadData {
		std::thread *threadHandle;
		uint32_t threadNum;
		TaskGraph *graph;
		bool isJobThread;

		void RunThread();
		void WaitForTasks();
	};

	static thread_local ThreadData *tl_threadData;

	bool TryRunTask(ThreadData *thread, bool allowJobs = true);
	void ExecTask(Task *task);
	bool HasTasks(ThreadData *thread);
	static ThreadData *GetThreadData();

	void WakeForNewTasks();
	void WakeForFinishedTasks();

	void WaitForFinishedTask();

	std::vector<ThreadData *> m_threads;

	// queue for short-lived high-priority tasks
	AsyncTaskQueueImpl *m_taskQueue;
	// queue of tasks to run on the main thread
	AsyncTaskQueueImpl *m_pinnedTasks;

	// implementation of the JobQueue interface for backwards compat
	// with the old JobQueue system
	TaskGraphJobQueueImpl *m_jobHandlerImpl;

	// queue for long-lived low-priority background jobs
	AsyncJobQueueImpl *m_jobQueue;
	AsyncJobQueueImpl *m_jobFinishedQueue;

	std::atomic<bool> m_isRunning;
	std::atomic<uint32_t> m_numAliveThreads;

	Semaphore m_newTasksSemaphore;
	Semaphore m_finishedTasksSemaphore;
};
