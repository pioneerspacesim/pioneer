// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "ModelViewer.h"
#include <cstdio>

enum RunMode {
	MODE_GAME,
	MODE_MODELVIEWER,
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

	switch (mode) {
		case MODE_GAME:
			Pi::Init();
			for (;;) Pi::Start();
			break;

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
			fprintf(stderr, "pioneer %s\n", version.c_str());
			break;
		}

		case MODE_USAGE_ERROR:
			fprintf(stderr, "pioneer: unknown mode %s\n", argv[1]);
			// fall through

		case MODE_USAGE:
			fprintf(stderr,
				"usage: pioneer [mode] [options...]\n"
				"available modes:\n"
				"    -game        [-g]     game (default)\n"
				"    -modelviewer [-mv]    model viewer\n"
				"    -version     [-v]     show version\n"
				"    -help        [-h,-?]  this help\n"
			);
			break;
	}

	return 0;
}
