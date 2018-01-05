// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "ModelViewer.h"
#include "Game.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/Galaxy.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>

enum RunMode {
	MODE_GAME,
	MODE_MODELVIEWER,
	MODE_GALAXYDUMP,
	MODE_SKIPMENU,
	MODE_VERSION,
	MODE_USAGE,
	MODE_USAGE_ERROR
};

int main(int argc, char** argv)
{
#ifdef PIONEER_PROFILER
	Profiler::detect( argc, argv );
#endif

	RunMode mode = MODE_GAME;
	std::string modeopt;

	if (argc > 1) {
		const char switchchar = argv[1][0];
		if (!(switchchar == '-' || switchchar == '/')) {
			mode = MODE_USAGE_ERROR;
			goto start;
		}

		modeopt = std::string(argv[1]).substr(1);

		if (modeopt == "game" || modeopt == "g") {
			mode = MODE_GAME;
			goto start;
		}

		if (modeopt == "modelviewer" || modeopt == "mv") {
			mode = MODE_MODELVIEWER;
			goto start;
		}

		if (modeopt == "galaxydump" || modeopt == "gd") {
			mode = MODE_GALAXYDUMP;
			goto start;
		}

		if (modeopt.find("skipmenu", 0, 8) != std::string::npos ||
			modeopt.find("sm", 0, 2) != std::string::npos)
		{
			mode = MODE_SKIPMENU;
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

	int pos = 2;
	long int radius = 4;
	long int sx = 0, sy = 0, sz = 0;
	std::string filename;
	int startPlanet = 0; // zero is off

	switch (mode) {
		case MODE_GALAXYDUMP: {
			if (argc < 3) {
				Output("pioneer: galaxy dump requires a filename\n");
				break;
			}
			filename = argv[pos];
			++pos;
			if (argc > pos) { // radius (optional)
				char* end = nullptr;
				radius = std::strtol(argv[pos], &end, 0);
				if (end == nullptr || *end != 0 || radius < 0 || radius > 10000) {
					Output("pioneer: invalid radius: %s\n", argv[pos]);
					break;
				}
				++pos;
			}
			if (argc > pos) { // center of dump (three comma separated coordinates, optional)
				char* end = nullptr;
				sx = std::strtol(argv[pos], &end, 0);
				if (end == nullptr || *end != ',' || sx < -10000 || sx > 10000) {
					Output("pioneer: invalid center: %s\n", argv[pos]);
					break;
				}
				sy = std::strtol(end + 1, &end, 0);
				if (end == nullptr || *end != ',' || sy < -10000 || sy > 10000) {
					Output("pioneer: invalid center: %s\n", argv[pos]);
					break;
				}
				sz = std::strtol(end + 1, &end, 0);
				if (end == nullptr || *end != 0 || sz < -10000 || sz > 10000) {
					Output("pioneer: invalid center: %s\n", argv[pos]);
					break;
				}
				++pos;
			}
			// fallthrough
		}
		case MODE_SKIPMENU: {
			// fallthrough protect
			if (mode == MODE_SKIPMENU)
			{
				// try to get start planet number
				std::vector<std::string> keyValue = SplitString(modeopt, "=");

				// if found value
				if (keyValue.size() == 2)
				{
					if (keyValue[1].empty())
					{
						startPlanet = 0;
					}
					else
					{
						startPlanet = static_cast<int>(StrToSInt64(keyValue[1]));
					}
				}
				// if value not exists - load on Earth
				else
					startPlanet = 1;

				// set usual mode
				mode = MODE_GAME;
			}
			// fallthrough
		}
		case MODE_GAME: {
			std::map<std::string,std::string> options;

			// if arguments more than parsed already
			if (argc > pos) {
				static const std::string delim("=");

				// for each argument
				for (; pos < argc; pos++) {
					const std::string arg(argv[pos]);
					std::vector<std::string> keyValue = SplitString(arg, "=");

					// if there no key and value || key is empty || value is empty
					if (keyValue.size() != 2 || keyValue[0].empty() || keyValue[1].empty()) {
						Output("malformed option: %s\n", arg.c_str());
						return 1;
					}

					// put key and value to config
					options[keyValue[0]] = keyValue[1];
				}
			}

			Pi::Init(options, mode == MODE_GALAXYDUMP);

			if (mode == MODE_GAME)
				for (;;) {
					Pi::Start(startPlanet);
					startPlanet = 0; // Reset the start planet when coming back to the menu
				}
			else if (mode == MODE_GALAXYDUMP) {
				FILE* file = filename == "-" ? stdout : fopen(filename.c_str(), "w");
				if (file == nullptr) {
					Output("pioneer: could not open \"%s\" for writing: %s\n", filename.c_str(), strerror(errno));
					break;
				}
				RefCountedPtr<Galaxy> galaxy = GalaxyGenerator::Create();
				galaxy->Dump(file, sx, sy, sz, radius);
				if (filename != "-" && fclose(file) != 0) {
					Output("pioneer: writing to \"%s\" failed: %s\n", filename.c_str(), strerror(errno));
				}
				Pi::Quit();
			}
			break;
		}

		case MODE_MODELVIEWER: {
			std::string modelName;
			if (argc > 2)
				modelName = argv[2];
			ModelViewer::Run(modelName);
			break;
		}

		case MODE_VERSION: {
			std::string version(PIONEER_VERSION);
			if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
			Output("pioneer %s\n", version.c_str());
			break;
		}

		case MODE_USAGE_ERROR:
			Output("pioneer: unknown mode %s\n", argv[1]);
			// fall through

		case MODE_USAGE:
			Output(
				"usage: pioneer [mode] [options...]\n"
				"available modes:\n"
				"    -game        [-g]     game (default)\n"
				"    -modelviewer [-mv]    model viewer\n"
				"    -galaxydump  [-gd]    galaxy dumper\n"
				"    -skipmenu    [-sm]    skip main menu\n"
				"    -skipmenu=N  [-sm=N]  skip main menu and load planet 'N' where N: number\n"
				"    -version     [-v]     show version\n"
				"    -help        [-h,-?]  this help\n"
			);
			break;
	}

	return 0;
}
