// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSystem.h"
#include "JobQueue.h"
#include "SDL_timer.h"
#include "atomic_queue/defs.h"
#include "core/TaskGraph.h"
#include "doctest/doctest.h"
#include "profiler/Profiler.h"

std::atomic<uint32_t> s_numExec = 0;

void busy_wait(uint32_t uSec = 500)
{
	Profiler::Clock clock{};
	clock.Start();
	while (clock.currentmilliseconds() * 1000 < uSec)
		atomic_queue::spin_loop_pause();
}

class TestTask : public Task {
public:
	void OnExecute(TaskRange) override
	{
		PROFILE_SCOPED()
		// printf("Task %p executed on thread %d!\n", this, TaskGraph::GetThreadNum());

		// run a workload for 50us
		busy_wait(50);
		s_numExec.fetch_add(1);
	}
};

class TestJob : public Job {
public:
	void OnRun() override
	{
		printf("Job %p executed on thread %d!\n", this, TaskGraph::GetThreadNum());
		// run a workload for 500us
		busy_wait(500);
	}

	void OnFinish() override
	{
		printf("TestJob %p completed!\n", this);
	}

	void OnCancel() override
	{
		printf("TestJob %p cancelled!\n", this);
	}
};

TEST_CASE("Task Graph")
{
	TaskGraph *graph = new TaskGraph();
	graph->SetWorkerThreads(3);
	s_numExec = 0;

	SUBCASE("Single Task")
	{
		Task *newTask = new TestTask();
		graph->QueueTask(newTask);

		newTask = new TestTask();
		graph->QueueTask(newTask);
		SDL_Delay(100);
		CHECK(s_numExec.load() == 2);
	}

	SUBCASE("TaskSet")
	{
		TaskSet *set = new TaskSet();
		for (uint32_t idx = 0; idx < 64; idx++) {
			set->AddTask(new TestTask());
		}

		TaskSet::Handle handle = graph->QueueTaskSet(set);
		while (!handle.IsComplete())
			SDL_Delay(100);
		graph->CompleteTaskSet(handle);
		CHECK(s_numExec.load() == 64);
	}

	SUBCASE("Wait for Task Set")
	{
		TaskSet *set = new TaskSet();
		for (uint32_t idx = 0; idx < 256; idx++) {
			set->AddTask(new TestTask());
		}

		TaskSet::Handle handle = graph->QueueTaskSet(set);
		graph->WaitForTaskSet(handle);

		CHECK(s_numExec.load() == 256);
	}

	SUBCASE("Pinned Tasks")
	{
		TaskSet *set = new TaskSet();
		for (uint32_t idx = 0; idx < 64; idx++) {
			set->AddTask(new TestTask());
		}

		TaskSet::Handle handle = graph->QueueTaskSetPinned(set);
		while (!handle.IsComplete())
			graph->RunPinnedTasks();

		CHECK(s_numExec.load() == 64);
	}

	SUBCASE("Wait for Pinned Tasks")
	{
		TaskSet *set = new TaskSet();
		for (uint32_t idx = 0; idx < 64; idx++) {
			set->AddTask(new TestTask());
		}

		TaskSet::Handle handle = graph->QueueTaskSetPinned(set);
		graph->WaitForTaskSet(handle);

		CHECK(s_numExec.load() == 64);
	}

	SUBCASE("JobQueue Replacement")
	{
		Job *nj = new TestJob();
		Job *nj2 = new TestJob();

		Job::Handle h1 = graph->GetJobQueue()->Queue(nj);
		graph->GetJobQueue()->Queue(nj2);

		printf("Finishing jobs:\n");
		while (h1.HasJob())
			graph->GetJobQueue()->FinishJobs();
	}

	SUBCASE("Wait on Other Threads")
	{
		Profiler::reset();
		TaskSet *set1 = new TaskSet();

		for (uint32_t idx = 0; idx < 4; idx++) {
			set1->AddTaskLambda({ 0, 32 }, [=](TaskRange r) {
				PROFILE_SCOPED_DESC("Launcher Task")

				TaskSet *setN = new TaskSet();
				for (uint32_t idx = r.begin; idx < r.end; idx++)
					setN->AddTask(new TestTask());

				auto handle = graph->QueueTaskSet(setN);
				graph->WaitForTaskSet(handle);
				printf("Job %d finished waiting on thread %d\n", idx, graph->GetThreadNum());
			});
		}

		auto handle = graph->QueueTaskSet(set1);
		graph->WaitForTaskSet(handle);
		CHECK(s_numExec.load() == 128);

		// const std::string path = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
		// Profiler::dumpzones(path.c_str());
	}

	SUBCASE("Add Pinned Tasks from Other Threads")
	{
		Profiler::reset();
		TaskSet *set1 = new TaskSet();

		for (uint32_t idx = 0; idx < 4; idx++) {
			set1->AddTaskLambda({}, [=](TaskRange) {
				PROFILE_SCOPED_DESC("Launcher Task")

				graph->QueueTaskPinned(new TestTask());
				busy_wait(500);
			});
		}

		auto handle = graph->QueueTaskSet(set1);
		busy_wait(50);
		graph->WaitForTaskSet(handle);

		// const std::string path = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
		// Profiler::dumpzones(path.c_str());
	}

	/*
	SUBCASE("Profile Task Set") {
		Profiler::reset();

		PROFILE_SCOPED_DESC("Profile Task Set")
		Profiler::Clock clock {};
		clock.Start();

		for (uint32_t iter = 0; iter < 256; iter++) {
			PROFILE_SCOPED_DESC("WaitForTaskSet")
			TaskSet *set = new TaskSet();
			for (uint32_t idx = 0; idx < 256; idx++) {
				set->AddTask(new TestTask());
			}

			TaskSet::Handle handle = graph->QueueTaskSet(set);
			graph->WaitForTaskSet(handle);

		}

		clock.Stop();
		printf("TaskSet Profile completed in %f ms\n", clock.milliseconds());
		CHECK(s_numExec.load() == 256*256);

		const std::string path = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
		Profiler::dumpzones(path.c_str());
	}
	//*/

	delete graph;
}
