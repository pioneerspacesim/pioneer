// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "buildopts.h"
#include "core/Log.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>

#include "scenegraph/SceneGraph.h"

#include "FileSystem.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "JobQueue.h"
#include "ModManager.h"
#include "StringF.h"
#include "core/OS.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "graphics/dummy/RendererDummy.h"
#include "scenegraph/BinaryConverter.h"
#include "scenegraph/DumpVisitor.h"
#include "scenegraph/FindNodeVisitor.h"
#include <sstream>
#include <SDL.h>

std::unique_ptr<GameConfig> s_config;
std::unique_ptr<Graphics::Renderer> s_renderer;

//#define USES_THREADS
#ifdef USES_THREADS
std::unique_ptr<AsyncJobQueue> asyncJobQueue;
#endif

static const std::string s_dummyPath("");

// fwd decl'
void RunCompiler(const std::string &modelName, const std::string &filepath, const bool bInPlace);

// ********************************************************************************
// Overloaded PureJob class to handle compiling each model
// ********************************************************************************
class CompileJob : public Job {
public:
	CompileJob(){};
	CompileJob(const std::string &name, const std::string &path, const bool inPlace) :
		m_name(name),
		m_path(path),
		m_inPlace(inPlace) {}

	virtual void OnRun() override final { RunCompiler(m_name, m_path, m_inPlace); } // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void OnFinish() override final {}
	virtual void OnCancel() override final {}

protected:
	std::string m_name;
	std::string m_path;
	bool m_inPlace;
};

// ********************************************************************************
// functions
// ********************************************************************************
void SetupRenderer()
{
	PROFILE_SCOPED()
	s_config.reset(new GameConfig);

	Log::GetLog()->SetLogFile("modelcompiler.log");

	//init components
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
	static const Uint32 sdl_init_nothing = 0;
	if (SDL_Init(sdl_init_nothing) < 0)
		Error("SDL initialization failed: %s\n", SDL_GetError());

	ModManager::Init();

	Graphics::RendererDummy::RegisterRenderer();

	//video
	Graphics::Settings videoSettings = {};
	videoSettings.rendererType = Graphics::RENDERER_DUMMY;
	videoSettings.width = s_config->Int("ScrWidth");
	videoSettings.height = s_config->Int("ScrHeight");
	videoSettings.fullscreen = false;
	videoSettings.hidden = true;
	videoSettings.requestedSamples = s_config->Int("AntiAliasingMode");
	videoSettings.vsync = false;
	videoSettings.useTextureCompression = true;
	videoSettings.useAnisotropicFiltering = true;
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = "Model Compiler";
	s_renderer.reset(Graphics::Init(videoSettings));

#ifdef USES_THREADS
	// get threads up
	Uint32 numThreads = s_config->Int("WorkerThreads");
	const int numCores = OS::GetNumCores();
	assert(numCores > 0);
	if (numThreads == 0)
		numThreads = std::max(Uint32(numCores), 1U); // this is a tool, we can use all of the cores for processing unlike Pioneer
	asyncJobQueue.reset(new AsyncJobQueue(numThreads));
	Output("started %d worker threads\n", numThreads);
#endif
}

void RunCompiler(const std::string &modelName, const std::string &filepath, const bool bInPlace)
{
	PROFILE_SCOPED()
	Profiler::Timer timer;
	timer.Start();
	Output("\n---\nStarting compiler for (%s)\n", modelName.c_str());

	//load the current model in a pristine state (no navlights, shields...)
	//and then save it into binary
	std::unique_ptr<SceneGraph::Model> model;
	try {
		SceneGraph::Loader ld(s_renderer.get(), true, false);
		model.reset(ld.LoadModel(modelName));
		//dump warnings
		for (std::vector<std::string>::const_iterator it = ld.GetLogMessages().begin();
			 it != ld.GetLogMessages().end(); ++it) {
			Output("%s\n", (*it).c_str());
		}
	} catch (...) {
		//minimal error handling, this is not expected to happen since we got this far.
		return;
	}

	try {
		const std::string DataPath = FileSystem::NormalisePath(filepath.substr(0, filepath.size() - 6));
		SceneGraph::BinaryConverter bc(s_renderer.get());
		bc.Save(modelName, DataPath, model.get(), bInPlace);
	} catch (const CouldNotOpenFileException &) {
	} catch (const CouldNotWriteToFileException &) {
	}

	timer.Stop();
	Output("Compiling \"%s\" took: %lf\n", modelName.c_str(), timer.millicycles());
}

// ********************************************************************************
// functions
// ********************************************************************************
enum RunMode {
	MODE_MODELCOMPILER = 0,
	MODE_MODELBATCHEXPORT,
	MODE_VERSION,
	MODE_USAGE,
	MODE_USAGE_ERROR
};

static FileSystem::FileSourceFS customDataDir(".");

extern "C" int main(int argc, char **argv)
{
#ifdef PIONEER_PROFILER
	Profiler::detect(argc, argv);
#endif

	RunMode mode = MODE_MODELCOMPILER;

	if (argc > 1) {
		const char switchchar = argv[1][0];
		if (!(switchchar == '-' || switchchar == '/')) {
			mode = MODE_USAGE_ERROR;
			goto start;
		}

		const std::string modeopt(std::string(argv[1]).substr(1));

		if (modeopt == "compile" || modeopt == "c") {
			mode = MODE_MODELCOMPILER;
			goto start;
		}

		if (modeopt == "batch" || modeopt == "b") {
			mode = MODE_MODELBATCHEXPORT;
			goto start;
		}

		if (modeopt == "version" || modeopt == "v") {
			mode = MODE_VERSION;
			goto start;
		}

		if (modeopt == "help" || modeopt == "h" || modeopt == "?") {
			mode = MODE_USAGE;
			goto start;
		}

		mode = MODE_USAGE_ERROR;
	}

start:

	// Init here since we'll need it for both batch and RunCompiler modes.
	FileSystem::Init();
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
#ifdef PIONEER_PROFILER
	FileSystem::userFiles.MakeDirectory("profiler");
#endif

	// what mode are we in?
	switch (mode) {
	case MODE_MODELCOMPILER: {
		std::string modelName;
		std::string filePath;
		if (argc > 2) {
			filePath = modelName = argv[2];
			// determine if we're meant to be writing these in the source directory
			bool isInPlace = false;
			if (argc > 3) {
				std::string arg3 = argv[3];
				isInPlace = (arg3 == "inplace" || arg3 == "true");

				// find all of the models
				FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
				for (FileSystem::FileEnumerator files(fileSource, "models", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
					const FileSystem::FileInfo &info = files.Current();
					const std::string &fpath = info.GetPath();

					//check it's the expected type
					if (info.IsFile()) {
						if (ends_with_ci(fpath, ".model")) { // store the path for ".model" files
							const std::string shortname(info.GetName().substr(0, info.GetName().size() - 6));
							if (shortname == modelName) {
								filePath = fpath;
								break;
							}
						}
					}
				}
			}
			SetupRenderer();
			RunCompiler(modelName, filePath, isInPlace);
		}
		break;
	}

	case MODE_MODELBATCHEXPORT: {
		// determine if we're meant to be writing these in the source directory
		bool isInPlace = false;
		if (argc > 2) {
			std::string arg2 = argv[2];
			isInPlace = (arg2 == "inplace" || arg2 == "true");

			if (!isInPlace && !arg2.empty()) {
				customDataDir = FileSystem::FileSourceFS(arg2);
				FileSystem::gameDataFiles.AppendSource(&customDataDir);
				isInPlace = true;
			}
		}

		// find all of the models
		std::vector<std::pair<std::string, std::string>> list_model;
		FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
		for (FileSystem::FileEnumerator files(fileSource, "models", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			const std::string &fpath = info.GetPath();

			//check it's the expected type
			if (info.IsFile()) {
				if (ends_with_ci(fpath, ".model")) { // store the path for ".model" files
					list_model.push_back(std::make_pair(info.GetName().substr(0, info.GetName().size() - 6), fpath));
				}
			}
		}

		SetupRenderer();

#ifndef USES_THREADS
		for (auto &modelName : list_model) {
			RunCompiler(modelName.first, modelName.second, isInPlace);
		}
#else
		std::deque<Job::Handle> handles;
		for (auto &modelName : list_model) {
			handles.push_back(asyncJobQueue->Queue(new CompileJob(modelName.first, modelName.second, isInPlace)));
		}

		while (true) {
			asyncJobQueue->FinishJobs();
			bool hasJobs = false;
			for (auto &handle : handles)
				hasJobs |= handle.HasJob();

			if (!hasJobs)
				break;
		}
#endif
		break;
	}

	case MODE_VERSION: {
		std::string version(PIONEER_VERSION);
		if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
		Output("modelcompiler %s\n", version.c_str());
		break;
	}

	case MODE_USAGE_ERROR:
		Output("modelcompiler: unknown mode %s\n", argv[1]);
		// fall through

	case MODE_USAGE:
		Output(
			"usage: modelcompiler [mode] [options...]\n"
			"available modes:\n"
			"    -compile          [-c ...]          model compiler\n"
			"    -compile inplace  [-c ... inplace]  model compiler\n"
			"    -batch            [-b]              batch mode output into users home/Pioneer directory\n"
			"    -batch inplace    [-b inplace]      batch mode output into the source folder\n"
			"    -version          [-v]              show version\n"
			"    -help             [-h,-?]           this help\n");
		break;
	}

#ifdef PIONEER_PROFILER
	Profiler::dumphtml(FileSystem::JoinPathBelow(FileSystem::GetUserDir(), "profiler").c_str());
#endif

	Graphics::Uninit();
	SDL_Quit();
	FileSystem::Uninit();
#ifdef USES_THREADS
	asyncJobQueue.reset();
#endif
	//exit(0);

	return 0;
}
