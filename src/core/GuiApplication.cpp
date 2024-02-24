// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GuiApplication.h"
#include "IniConfig.h"
#include "Input.h"
#include "OS.h"

#include "SDL.h"
#include "SDL_video.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/RenderState.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "pigui/PiGui.h"
#include "pigui/PiGuiRenderer.h"
#include "profiler/Profiler.h"
#include "versioningInfo.h"

GuiApplication::GuiApplication(std::string title) :
	Application(), m_applicationTitle(title)
{}

GuiApplication::~GuiApplication()
{ }

void GuiApplication::BeginFrame()
{
	PROFILE_SCOPED()

	m_renderer->SetRenderTarget(m_renderTarget.get());
	m_renderer->SetViewport({ 0, 0, m_renderer->GetWindowWidth(), m_renderer->GetWindowHeight() });
	m_renderer->ClearScreen();

	m_renderer->BeginFrame();
	m_input->NewFrame();
}

void GuiApplication::EndFrame()
{
	PROFILE_SCOPED()

	m_renderer->FlushCommandBuffers();
	m_renderer->EndFrame();
	m_renderer->SwapBuffers();
}

Graphics::RenderTarget *GuiApplication::CreateRenderTarget(const Graphics::Settings &settings)
{
	Graphics::RenderTargetDesc rtDesc = {
		uint16_t(settings.width), uint16_t(settings.height),
		Graphics::TEXTURE_RGBA_8888,
		Graphics::TEXTURE_DEPTH, true,
		uint16_t(settings.requestedSamples)
	};

	return m_renderer->CreateRenderTarget(rtDesc);
}

void GuiApplication::OnWindowResized()
{
	// Let the renderer determine the new size of the backbuffer
	m_renderer->OnWindowResized();

	// Check to see if we need to resize the render target (events are flushed after BeginFrame)
	Graphics::RenderTargetDesc rtDesc = m_renderTarget->GetDesc();
	int width = m_renderer->GetWindowWidth();
	int height = m_renderer->GetWindowHeight();

	// To avoid a one-frame delay, we need to recreate the render target now, rather than next frame
	if (width != m_renderTarget->GetDesc().width || height != m_renderTarget->GetDesc().height) {
		// Flush all commands using the prior render target
		m_renderer->FlushCommandBuffers();
		m_renderer->SetRenderTarget(nullptr);

		// Copy the existing render target settings (MSAA etc.) and resize
		rtDesc.width = width;
		rtDesc.height = height;
		m_renderTarget.reset(m_renderer->CreateRenderTarget(rtDesc));

		// Setup the new render target for rendering
		m_renderer->SetRenderTarget(m_renderTarget.get());
		m_renderer->SetViewport({ 0, 0, width, height });
		m_renderer->ClearScreen();
	}
}

void GuiApplication::PollEvents()
{
	PROFILE_SCOPED()
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			RequestQuit();
		}

		if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_RESIZED)) {
			OnWindowResized();
		}

		m_pigui->ProcessEvent(&event);

		// Input system takes priority over mouse events when capturing the mouse
		if (PiGui::WantCaptureMouse() && !m_input->IsCapturingMouse()) {
			// don't process mouse event any further, imgui already handled it
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
			case SDL_MOUSEMOTION:
				continue;
			default: break;
			}
		}
		if (PiGui::WantCaptureKeyboard()) {
			// don't process keyboard event any further, imgui already handled it
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
				continue;
			default: break;
			}
		}

		m_input->HandleSDLEvent(event);
	}
}

void GuiApplication::DispatchEvents()
{
	m_input->DispatchEvents();
}

void GuiApplication::HandleEvents()
{
	PollEvents();
	DispatchEvents();
}

void GuiApplication::SetupProfiler(IniConfig *config)
{
	// Setup common profiling parameters from the config file
	bool profileSlow   = config->Int("ProfileSlowFrames", 0);
	bool profileZones  = config->Int("ProfilerZoneOutput", 0);
	bool profileTraces = config->Int("ProfilerTraceOutput", 0);

	SetProfilerPath("profiler/");
	SetProfileSlowFrames(profileSlow);
	SetProfileZones(profileZones || profileTraces);
	SetProfileTrace(profileTraces);
}

Graphics::Renderer *GuiApplication::StartupRenderer(IniConfig *config, bool hidden, bool resizable)
{
	PROFILE_SCOPED()

	// Initialize SDL
	PROFILE_START_DESC("SDL_Init")
	Uint32 sdlInitFlags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
	if (SDL_Init(sdlInitFlags) < 0) {
		Error("SDL initialization failed: %s\n", SDL_GetError());
	}
	PROFILE_STOP()

	OutputVersioningInfo();

	// determine what renderer we should use, default to Opengl 3.x
	const std::string rendererName = config->String("RendererName", Graphics::RendererNameFromType(Graphics::RENDERER_OPENGL_3x));
	// if we add new renderer types, make sure to update this logic
	Graphics::RendererType rType = Graphics::RENDERER_OPENGL_3x;

	Graphics::Settings videoSettings = {};
	videoSettings.rendererType = rType;
	videoSettings.width = config->Int("ScrWidth");
	videoSettings.height = config->Int("ScrHeight");
	videoSettings.fullscreen = (config->Int("StartFullscreen") != 0);
	videoSettings.canBeResized = resizable;
	videoSettings.hidden = hidden;
	videoSettings.requestedSamples = config->Int("AntiAliasingMode");
	videoSettings.vsync = (config->Int("VSync") != 0);
	videoSettings.useTextureCompression = (config->Int("UseTextureCompression") != 0);
	videoSettings.useAnisotropicFiltering = (config->Int("UseAnisotropicFiltering") != 0);
	videoSettings.enableDebugMessages = (config->Int("EnableGLDebug") != 0);
	videoSettings.gl3ForwardCompatible = (config->Int("GL3ForwardCompatible") != 0);
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = m_applicationTitle.c_str();

	m_renderer.reset(Graphics::Init(videoSettings));
	m_renderTarget.reset(CreateRenderTarget(videoSettings));

	m_settings = videoSettings;

	return m_renderer.get();
}

void GuiApplication::ShutdownRenderer()
{
	PROFILE_SCOPED()
	m_renderTarget.reset();
	m_renderer.reset();

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

Input::Manager *GuiApplication::StartupInput(IniConfig *config)
{
	PROFILE_SCOPED()
	m_input.reset(new Input::Manager(config, m_renderer->GetSDLWindow()));

	return m_input.get();
}

void GuiApplication::ShutdownInput()
{
	PROFILE_SCOPED()
	m_input.reset();
}

PiGui::Instance *GuiApplication::StartupPiGui()
{
	PROFILE_SCOPED()
	m_pigui.Reset(new PiGui::Instance(this));
	m_pigui->Init(GetRenderer());
	return m_pigui.Get();
}

void GuiApplication::ShutdownPiGui()
{
	PROFILE_SCOPED()
	m_pigui->Uninit();
	m_pigui.Reset();
}
