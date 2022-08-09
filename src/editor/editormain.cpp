// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorApp.h"

#include "argh/argh.h"

#include <memory>

extern "C" int main(int argc, const char **argv) {
	argh::parser cmdline(argc, argv);

	Editor::EditorApp *app = Editor::EditorApp::Get(); // instance the editor application

	app->Startup();
	app->Initialize(cmdline);

	app->Run();

	app->Shutdown();
}
