// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Application.h"
#include "GameConfig.h"
#include "RefCounted.h"

#include "graphics/RenderState.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"

class GuiApplication : public Application {
public:
	GuiApplication(std::string title) :
		Application(), m_applicationTitle(title)
	{}

protected:
	Graphics::Renderer *GetRenderer() { return m_renderer.get(); }

	// Called at the end of the frame automatically, blits the RT onto the application
	// framebuffer
	void DrawRenderTarget();

	// Call this from your Startup() method
	Graphics::Renderer *StartupRenderer(const GameConfig *config, bool hidden = false);

	// Call this from your Shutdown() method
	void ShutdownRenderer();

	// Hook to bind the RT and clear the screen.
	// If you override BeginFrame, make sure you call this.
	void BeginFrame() override;

	// Hook to end the frame and draw to the application framebuffer.
	// If you override EndFrame, make sure you call this.
	void EndFrame() override;

private:
	Graphics::RenderTarget *CreateRenderTarget(const Graphics::Settings &settings);

	std::string m_applicationTitle;

	std::unique_ptr<Graphics::Renderer> m_renderer;
	std::unique_ptr<Graphics::RenderTarget> m_renderTarget;
	std::unique_ptr<Graphics::RenderState> m_renderState;
};
