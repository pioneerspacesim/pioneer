// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TaskGraph.h"

#include "JobQueue.h"
#include "SDL_timer.h"
#include "core/StringName.h"
#include "fmt/format.h"
#include "profiler/Profiler.h"
#include <atomic_queue/atomic_queue.h>
#include <atomic>

static constexpr size_t MAX_TASK_QUEUE_SIZE = 1024;
static constexpr size_t MAX_JOB_QUEUE_SIZE = 1024;

static constexpr size_t MAX_PINNED_TASKS_STEP = 8;
static constexpr size_t MAX_SPIN_COUNT = 10000;

class AsyncTaskQueueImpl : public atomic_queue::AtomicQueue2<Task *, MAX_TASK_QUEUE_SIZE> {};
class AsyncJobQueueImpl : public atomic_queue::AtomicQueue2<Job *, MAX_JOB_QUEUE_SIZE> {};

// =============================================================================

// implementation structure to maintain backwards compatibility with existing Job API
class TaskGraphJobQueueImpl : public JobQueue {
public:
	TaskGraphJobQueueImpl(TaskGraph *graph) :
		m_graph(graph) {}

	virtual Job::Handle Queue(Job *job, JobClient *client) override;
	virtual void Cancel(Job *job) override;
	virtual Uint32 FinishJobs() override;

	TaskGraph *m_graph;
};

Job::Handle TaskGraphJobQueueImpl::Queue(Job *job, JobClient *client)
{
	Job::Handle handle(job, this, client);

	m_graph->m_jobQueue->push(job);

	std::atomic_thread_fence(std::memory_order_release);
	m_graph->WakeForNewTasks();

	return handle;
}

void TaskGraphJobQueueImpl::Cancel(Job *job)
{
	job->cancelled.store(true, std::memory_order_release);
	job->m_handle.store(nullptr, std::memory_order_release);
}

Uint32 TaskGraphJobQueueImpl::FinishJobs()
{
	uint32_t numFinished = 0;
	Job *job = nullptr;

	while (m_graph->m_jobFinishedQueue->try_pop(job)) {
		job->UnlinkHandle();
		if (job->cancelled.load(std::memory_order_relaxed)) {
			job->OnCancel();
		} else {
			job->OnFinish();
			numFinished++;
		}

		delete job;
	}

	return numFinished;
}

// =============================================================================

void Task::SetOwner(CompleteNotifier *comp)
{
	// handle the (unlikely) case in which a task is being moved from an owner
	if (m_owner)
		m_owner->m_dependants.fetch_sub(1);

	m_owner = comp;
	comp->m_dependants.fetch_add(1);
}

bool TaskSet::AddTask(Task *task)
{
	if (m_executing)
		return false;

	m_tasks.push_back(task);
	task->SetOwner(this);
	return true;
}

// =============================================================================

TaskGraph::TaskGraph() :
	m_threads(),
	m_taskQueue(new AsyncTaskQueueImpl()),
	m_pinnedTasks(new AsyncTaskQueueImpl()),
	m_jobHandlerImpl(new TaskGraphJobQueueImpl(this)),
	m_jobQueue(new AsyncJobQueueImpl()),
	m_jobFinishedQueue(new AsyncJobQueueImpl()),
	m_isRunning(true),
	m_numAliveThreads(0)
{
	// initialize the main thread data
	SetWorkerThreads(0);
}

TaskGraph::~TaskGraph()
{
	m_isRunning.store(false);
	m_numAliveThreads.fetch_sub(1);

	// Shutdown threads, wait for the number of running threads to hit zeron
	while (m_numAliveThreads.load(std::memory_order_acquire)) {
		WakeForNewTasks();
		SDL_Delay(10); // force sleep on this thread
	}

	// once all threads have finished, join them and clean up their allocations
	for (size_t idx = 0; idx < m_threads.size(); idx++) {
		if (m_threads[idx]->threadHandle) {
			m_threads[idx]->threadHandle->join();
			delete m_threads[idx]->threadHandle;
		}

		delete m_threads[idx];
	}

	// Cleanup leftover task objects
	Task *task = nullptr;
	while (m_taskQueue->try_pop(task)) {
		delete task;
	}

	// clean up pinned tasks from this thread
	while (m_pinnedTasks->try_pop(task)) {
		delete task;
	}

	// cleanup leftover job objects
	Job *job = nullptr;
	while (m_jobQueue->try_pop(job)) {
		job->UnlinkHandle();
		job->OnCancel();
		delete job;
	}

	// cleanup all jobs that are waiting for the main thread
	while (m_jobFinishedQueue->try_pop(job)) {
		job->UnlinkHandle();
		job->OnCancel();
		delete job;
	}

	delete m_taskQueue;
	delete m_pinnedTasks;
	delete m_jobHandlerImpl;
	delete m_jobQueue;
	delete m_jobFinishedQueue;
}

uint32_t TaskGraph::GetNumWorkerThreads() const
{
	// m_threads[0] is the thread entry for the main thread
	return m_threads.size() - 1;
}

void TaskGraph::SetWorkerThreads(uint32_t numThreads)
{
	// numThreads + 1 because we have an implicit thread entry for the "main" thread
	if (numThreads + 1 <= m_threads.size())
		return;

	uint32_t existingSize = m_threads.size();
	m_threads.resize(numThreads + 1);

	for (uint32_t idx = existingSize; idx < numThreads + 1; idx++) {
		ThreadData *thr = new ThreadData();
		m_threads[idx] = thr;

		thr->threadNum = idx;
		thr->graph = this;
		thr->isJobThread = idx > 0;
		m_numAliveThreads.fetch_add(1, std::memory_order_release);
		if (idx > 0)
			thr->threadHandle = new std::thread(&ThreadData::RunThread, thr);
	}

	// setup threadlocal data for the "main" thread here
	tl_threadData = m_threads[0];
	assert(m_numAliveThreads == numThreads + 1);
}

TaskSet::Handle TaskGraph::QueueTaskSet(TaskSet *taskSet)
{
	taskSet->m_executing = true;
	for (auto task : taskSet->m_tasks) {
		m_taskQueue->push(task);
	}

	std::atomic_thread_fence(std::memory_order_release);
	WakeForNewTasks();
	return TaskSet::Handle(taskSet);
}

void TaskGraph::QueueTask(Task *task)
{
	m_taskQueue->push(task);

	// wake all threads that can run this task
	WakeForNewTasks();
}

TaskSet::Handle TaskGraph::QueueTaskSetPinned(TaskSet *taskSet)
{
	taskSet->m_executing = true;
	for (auto task : taskSet->m_tasks) {
		// Check that the graph is still running to avoid a potential deadlock
		// during shutdown.
		while (!m_pinnedTasks->try_push(task) && m_isRunning.load(std::memory_order_relaxed)) {
			WakeForNewTasks();

			// If we're the main thread, we deadlock if we wait on pinned tasks
			// to complete; instead, take the option that makes forward
			// progress and immediately run a pending pinned task to free a
			// queue slot to push this task.
			if (GetThreadData()->threadNum == 0)
				RunPinnedTasks();
			else
				WaitForFinishedTask();
		}
	}

	WakeForNewTasks();
	return TaskSet::Handle(taskSet);
}

void TaskGraph::QueueTaskPinned(Task *task)
{
	m_pinnedTasks->push(task);

	// wakeup any threads blocked on finished tasks (e.g. main thread)
	WakeForFinishedTasks();
}

void TaskGraph::WaitForTaskSet(TaskSet::Handle &setHandle)
{
	PROFILE_SCOPED()

	if (!setHandle.m_set)
		return;

	// if the currently running thread can't accomplish anything until the
	// TaskSet has finished executing, this thread is implicitly free to assist
	// in executing the TaskSet to minimize overall latency.
	uint32_t spinCount = 0;
	while (m_isRunning && !setHandle.IsComplete()) {
		// We don't want to run any background Jobs during this loop as they
		// cannot contribute towards the goal of completing the TaskSet
		if (!TryRunTask(m_threads[0], false)) {
			if (++spinCount > MAX_SPIN_COUNT) {
				WaitForFinishedTask();
			} else {
				atomic_queue::spin_loop_pause();
			}
		} else {
			spinCount = 0;
		}
	}

	if (setHandle.IsComplete())
		CompleteTaskSet(setHandle);
}

bool TaskGraph::CompleteTaskSet(TaskSet::Handle &setHandle)
{
	if (!setHandle.m_set)
		return true;

	if (!setHandle.m_set->IsComplete())
		return false;

	for (auto task : setHandle.m_set->m_tasks) {
		task->OnComplete();
		delete task;
	}

	delete setHandle.m_set;
	setHandle.m_set = nullptr;
	return true;
}

void TaskGraph::RunPinnedTasks()
{
	PROFILE_SCOPED()

	if (!m_pinnedTasks->was_size())
		return;

	for (uint32_t idx = 0; idx < MAX_PINNED_TASKS_STEP; idx++) {
		Task *task = nullptr;
		if (!m_pinnedTasks->try_pop(task))
			break;

		ExecTask(task);
	}
}

JobQueue *TaskGraph::GetJobQueue()
{
	return m_jobHandlerImpl;
}

uint32_t TaskGraph::GetThreadNum()
{
	return GetThreadData()->threadNum;
}

// =============================================================================

thread_local std::string tl_threadName;
thread_local TaskGraph::ThreadData *TaskGraph::tl_threadData;
void TaskGraph::ThreadData::RunThread()
{
	tl_threadData = this;
	tl_threadName = fmt::format("Thread {}", threadNum);
	Profiler::threadenter(tl_threadName.c_str());

	// Worker threads pull Tasks / Jobs off of a central queue that all
	// threads can push to - this requires the use of a bulletproof atomic
	// queue implementation for speed and reliability.

	// We also spin-wait for a fairly long time retrying for more work items
	// to optimize the case in which there are many threaded work items being
	// added to the system.

	uint32_t spinCount = 0;
	while (graph->m_isRunning.load(std::memory_order_relaxed)) {
		if (!graph->TryRunTask(this)) {
			if (++spinCount > MAX_SPIN_COUNT) {
				WaitForTasks();
			} else {
				atomic_queue::spin_loop_pause();
			}
		} else {
			spinCount = 0;

			// reclaim used space in this thread's StringTable periodically
			StringTable::Get()->Reclaim();
		}
	}

	graph->m_numAliveThreads.fetch_sub(1, std::memory_order_release);
	Profiler::threadexit();
}

TaskGraph::ThreadData *TaskGraph::GetThreadData()
{
	return tl_threadData;
}

void TaskGraph::ThreadData::WaitForTasks()
{
	std::atomic_thread_fence(std::memory_order_acquire);
	if (graph->HasTasks(this))
		return;
	else {
		PROFILE_SCOPED_DESC("Idle Waiting for Tasks");
		graph->m_newTasksSemaphore.waitonly();
	}
}

bool TaskGraph::HasTasks(ThreadData *thread)
{
	if (thread->threadNum == 0 && m_pinnedTasks->was_size())
		return true;

	if (m_taskQueue->was_size())
		return true;

	if (thread->isJobThread && m_jobQueue->was_size())
		return true;

	return false;
}

// Try to run a task on this thread. Can be called from any thread
bool TaskGraph::TryRunTask(ThreadData *thread, bool allowJobs)
{
	Task *taskToRun = nullptr;
	bool hasTask = false;

	if (thread->threadNum == 0)
		hasTask = m_pinnedTasks->try_pop(taskToRun);

	if (!hasTask)
		hasTask = m_taskQueue->try_pop(taskToRun);

	if (hasTask) {
		ExecTask(taskToRun);
		return true;
	}

	Job *job = nullptr;
	if (allowJobs && thread->isJobThread && m_jobQueue->try_pop(job)) {
		if (!job->cancelled.load(std::memory_order_acquire))
			job->OnRun();
		m_jobFinishedQueue->push(job);

		return true;
	}

	return false;
}

void TaskGraph::ExecTask(Task *task)
{
	task->OnExecute(task->m_range);

	if (task->m_owner)
		task->m_owner->m_dependants.fetch_sub(1);
	else
		delete task;

	// Notify threads in WaitForTaskSet that a task has been completed, so they
	// can check if the set is finished and return.
	WakeForFinishedTasks();
}

void TaskGraph::WakeForNewTasks()
{
	PROFILE_SCOPED()
	// semaphore notify all threads waiting for new tasks
	m_newTasksSemaphore.signal(m_numAliveThreads.load(std::memory_order_relaxed));

	// also wakeup threads waiting on finished tasks
	// (currently only the main thread)
	WakeForFinishedTasks();
}

void TaskGraph::WakeForFinishedTasks()
{
	m_finishedTasksSemaphore.signal();
}

// waits in the main thread for any task to be completed
void TaskGraph::WaitForFinishedTask()
{
	m_finishedTasksSemaphore.wait();
}
