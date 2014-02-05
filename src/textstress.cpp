// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <cstdlib>
#include "SDL.h"
#include "FileSystem.h"
#include "OS.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "text/FontDescriptor.h"
#include "text/TextureFont.h"

static const int WIDTH  = 1024;
static const int HEIGHT = 768;

int main(int argc, char **argv)
{
	FileSystem::Init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		Output("sdl init failed: %s\n", SDL_GetError());
		exit(-1);
	}

	Graphics::Settings videoSettings;
	videoSettings.width = WIDTH;
	videoSettings.height = HEIGHT;
	videoSettings.fullscreen = false;
	videoSettings.requestedSamples = 0;
	videoSettings.vsync = false;
	videoSettings.useTextureCompression = false;
	videoSettings.enableDebugMessages = false;
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = "textstress";

	Graphics::Renderer *r = Graphics::Init(videoSettings);

	r->SetOrthographicProjection(0, WIDTH, HEIGHT, 0, -1, 1);
	r->SetTransform(matrix4x4f::Identity());
	r->SetClearColor(Color::BLACK);
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	r->SetDepthTest(false);

	const Text::FontDescriptor fontDesc(Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/UIFont.ini", "en"));
	Text::TextureFont *font = new Text::TextureFont(fontDesc, r);

	std::string str;
	for (int i = 33; i < 127; i++)
		str.push_back(i);

	while (1) {
		bool done = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				done = true;
		}
		if (done)
			break;

		font->RenderString(str.c_str(), rand()%(WIDTH*2)-WIDTH, rand()%HEIGHT, Color::WHITE);
		r->SwapBuffers();
	}

	delete font;
	delete r;

	SDL_Quit();

	exit(0);
}
