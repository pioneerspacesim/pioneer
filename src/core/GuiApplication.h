// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Application.h"
#include "RefCounted.h"
#include "SDL_events.h"

#include "graphics/Graphics.h"

class IniConfig;

namespace Input {
	class Manager;
}

namespace PiGui {
	class Instance;
}

namespace Graphics {
	class Renderer;
	class RenderTarget;
}

class GuiApplication : public Application {
public:
	GuiApplication(std::string title);
	~GuiApplication();

	Graphics::Renderer *GetRenderer() { return m_renderer.get(); }
	Input::Manager *GetInput() { return m_input.get(); }
	PiGui::Instance *GetPiGui() { return m_pigui.Get(); }

	Graphics::RenderTarget *GetRenderTarget() { return m_renderTarget.get(); }

	const Graphics::Settings &GetGraphicsSettings() { return m_settings; }

protected:

	// Call this from your OnStartup() method
	void SetupProfiler(IniConfig *config);

	// TODO: unify config handling, possibly make the config an Application member
	// Call this from your OnStartup() method
	Graphics::Renderer *StartupRenderer(IniConfig *config, bool hidden = false, bool resizable = false);

	// Call this from your OnStartup() method
	Input::Manager *StartupInput(IniConfig *config);

	// Call this from your OnStartup() method
	PiGui::Instance *StartupPiGui();

	// Call this from your OnShutdown() method
	void ShutdownRenderer();

	// Call this from your OnShutdown() method
	void ShutdownInput();

	// Call this from your OnShutdown() method
	void ShutdownPiGui();

	// Hook to bind the RT and clear the screen.
	// If you override BeginFrame, make sure you call this.
	void BeginFrame() override;

	// Hook to end the frame and draw to the application framebuffer.
	// If you override EndFrame, make sure you call this.
	void EndFrame() override;

	// Consume events from SDL and dispatch to pigui / input
	void HandleEvents();
	// the function from above actually calls the following two:
	void PollEvents();	   // writes events to ImGui and Input structures, also generate signals for low-level handling
	void DispatchEvents(); // triggers action-related signals, updates actions and axes

	// Override point to handle an application quit notification
	virtual void HandleQuit(SDL_QuitEvent &ev) { RequestQuit(); }

private:
	Graphics::RenderTarget *CreateRenderTarget(const Graphics::Settings &settings);
	void OnWindowResized();

	RefCountedPtr<PiGui::Instance> m_pigui;
	std::unique_ptr<Input::Manager> m_input;

	std::string m_applicationTitle;

	std::unique_ptr<Graphics::Renderer> m_renderer;
	std::unique_ptr<Graphics::RenderTarget> m_renderTarget;
	Graphics::Settings m_settings;
};
