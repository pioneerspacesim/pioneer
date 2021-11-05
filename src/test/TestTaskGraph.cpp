// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSystem.h"
#include "JobQueue.h"
#include "SDL_timer.h"
#include "atomic_queue/defs.h"
#include "core/TaskGraph.h"
#include "doctest/doctest.h"
#include "profiler/Profiler.h"

std::atomic<uint32_t> s_numExec = 0;

class TestTask : public Task {
public:
	void OnExecute(TaskRange) override
	{
		PROFILE_SCOPED()
		// printf("Task %p executed on thread %d!\n", this, TaskGraph::GetThreadNum());
		Profiler::Clock clock{};
		clock.Start();

		// run a workload for 50us
		while (clock.currentmilliseconds() < 0.050)
			atomic_queue::spin_loop_pause();
		s_numExec.fetch_add(1);
	}
};

class TestJob : public Job {
public:
	void OnRun() override
	{
		printf("Job %p executed on thread %d!\n", this, TaskGraph::GetThreadNum());
		Profiler::Clock clock{};
		clock.Start();
		// run a workload for 500us
		while (clock.currentmilliseconds() < 0.500)
			atomic_queue::spin_loop_pause();
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
	graph->SetWorkerThreads(2);
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

	/*
	SUBCASE("Profile Task Set") {
		PROFILE_SCOPED()
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

		const std::string path = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
		Profiler::dumpzones(path.c_str());
	}
	//*/

	delete graph;
}
