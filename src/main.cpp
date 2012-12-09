// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "NewModelViewer.h"

int main(int argc, char** argv)
{
	if (argc <= 1) {
		Pi::Init();
		for (;;) Pi::Start();
	} else {
		ModelViewer::Run(argc, argv);
	}
	return 0;
}
