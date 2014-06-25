// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>

#include "scenegraph/SceneGraph.h"

#include "FileSystem.h"
#include "GameConfig.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Drawables.h"
#include "graphics/VertexArray.h"
#include "scenegraph/DumpVisitor.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/BinaryConverter.h"
#include "OS.h"
#include "StringF.h"
#include "ModManager.h"
#include <sstream>

std::unique_ptr<GameConfig> s_config;
std::unique_ptr<Graphics::Renderer> s_renderer;

static const std::string s_dummyPath("");

void SetupRenderer()
{
	s_config.reset(new GameConfig);

	OS::RedirectStdio();

	//init components
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Error("SDL initialization failed: %s\n", SDL_GetError());

	ModManager::Init();

	//video
	Graphics::Settings videoSettings = {};
	videoSettings.width = s_config->Int("ScrWidth");
	videoSettings.height = s_config->Int("ScrHeight");
	videoSettings.fullscreen = false;
	videoSettings.hidden = true;
	videoSettings.requestedSamples = s_config->Int("AntiAliasingMode");
	videoSettings.vsync = false;
	videoSettings.useTextureCompression = true;
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = "Model Compiler";
	s_renderer.reset(Graphics::Init(videoSettings));
}

void RunCompiler(const std::string &modelName, const std::string &filepath)
{
	Profiler::Timer timer;
	timer.Start();

	//load the current model in a pristine state (no navlights, shields...)
	//and then save it into binary
	std::unique_ptr<SceneGraph::Model> model;
	try {
		SceneGraph::Loader ld(s_renderer.get(), false, false);
		model.reset(ld.LoadModel(modelName));
	} catch (...) {
		//minimal error handling, this is not expected to happen since we got this far.
		return;
	}

	try {
		if (filepath.empty()) {
			SceneGraph::BinaryConverter bc(s_renderer.get());
			bc.Save(modelName, model.get());
		} else {
			const std::string DataPath = FileSystem::NormalisePath(filepath.substr(0, filepath.size()-6));
			SceneGraph::BinaryConverter bc(s_renderer.get());
			bc.Save(modelName, DataPath, model.get());
		}
	} catch (const CouldNotOpenFileException&) {
	} catch (const CouldNotWriteToFileException&) {
	}

	timer.Stop();
	Output("Compiling \"%s\" took: %lf\n", modelName.c_str(), timer.millicycles());
}


enum RunMode {
	MODE_MODELCOMPILER=0,
	MODE_MODELBATCHEXPORT,
	MODE_VERSION,
	MODE_USAGE,
	MODE_USAGE_ERROR
};

int main(int argc, char** argv)
{
#ifdef PIONEER_PROFILER
	Profiler::detect( argc, argv );
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

	// what mode are we in?
	switch (mode) {
		case MODE_MODELCOMPILER: {
			std::string modelName;
			if (argc > 2) {
				modelName = argv[2];
				SetupRenderer();
				RunCompiler(modelName, s_dummyPath);
			}
			break;
		}

		case MODE_MODELBATCHEXPORT: {
			// determine if we're meant to be writing these in the source directory
			bool isInPlace = false;
			if (argc > 2) {
				std::string arg2 = argv[2];
				isInPlace = (arg2 == "inplace" || arg2 == "true");
			}

			// find all of the models
			std::vector<std::pair<std::string, std::string>> list_model;
			FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
			for (FileSystem::FileEnumerator files(fileSource, "models", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next())
			{
				const FileSystem::FileInfo &info = files.Current();
				const std::string &fpath = info.GetPath();

				//check it's the expected type
				if (info.IsFile()) {
					if (ends_with_ci(fpath, ".model")) {	// store the path for ".model" files
						list_model.push_back( std::make_pair(info.GetName().substr(0, info.GetName().size()-6), fpath) );
					}
				}
			}

			SetupRenderer();
			for (auto &modelName : list_model) {
				if(isInPlace) {
					RunCompiler(modelName.first, modelName.second);
				} else {
					RunCompiler(modelName.first, s_dummyPath);
				}
			}
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
				"    -compile          [-c]          model compiler\n"
				"    -batch            [-b]          batch mode output into users home/Pioneer directory\n"
				"    -batch inplace    [-b inplace]  batch mode output into the source folder\n"
				"    -version          [-v]          show version\n"
				"    -help             [-h,-?]       this help\n"
			);
			break;
	}

	return 0;
}
