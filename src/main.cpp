// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "ModelViewer.h"
#include "galaxy/Galaxy.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>

enum RunMode {
	MODE_GAME,
	MODE_MODELVIEWER,
	MODE_GALAXYDUMP,
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

	if (argc > 1) {
		const char switchchar = argv[1][0];
		if (!(switchchar == '-' || switchchar == '/')) {
			mode = MODE_USAGE_ERROR;
			goto start;
		}

		const std::string modeopt(std::string(argv[1]).substr(1));

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
		case MODE_GAME: {
			std::map<std::string,std::string> options;
			if (argc > pos) {
				static const std::string delim("=");
				for (; pos < argc; pos++) {
					const std::string arg(argv[pos]);
					size_t mid = arg.find_first_of(delim, 0);
					if (mid == std::string::npos || mid == 0 || mid == arg.length()-1) {
						Output("malformed option: %s\n", arg.c_str());
						return 1;
					}
					const std::string key(arg.substr(0, mid));
					const std::string val(arg.substr(mid+1, arg.length()));
					options[key] = val;
				}
			}
			Pi::Init(options, mode == MODE_GALAXYDUMP);
			if (mode == MODE_GAME)
				for (;;) Pi::Start();
			else if (mode == MODE_GALAXYDUMP) {
				FILE* file = filename == "-" ? stdout : fopen(filename.c_str(), "w");
				if (file == nullptr) {
					Output("pioneer: could not open \"%s\" for writing: %s\n", filename.c_str(), strerror(errno));
					break;
				}
				Pi::GetGalaxy()->Dump(file, sx, sy, sz, radius);
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
				"    -version     [-v]     show version\n"
				"    -help        [-h,-?]  this help\n"
			);
			break;
	}

	return 0;
}
