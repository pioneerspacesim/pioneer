// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _POST_PROCESSING_H_
#define _POST_PROCESSING_H_

#include "libs.h"
#include "Material.h"
#include "Drawables.h"

namespace Graphics {
	class Renderer;
	class PostProcess;
	class RenderTarget;
	class RenderState;

	class PostProcessing
	{
	public:
		explicit PostProcessing(Renderer *renderer);
		virtual ~PostProcessing();
	
		void BeginFrame();
		void EndFrame();
		void Run(PostProcess* pp = nullptr);
		void SetDeviceRT(RenderTarget* rt_device);

	private:
		PostProcessing(const PostProcessing&);
		PostProcessing& operator=(const PostProcessing&);

		void Init();

		bool DrawFullscreenQuad(Material *mat, RenderState *state, bool clear_rt = false);
		bool DrawFullscreenQuad(Texture *texture, RenderState *state, bool clear_rt = false);
		
		std::unique_ptr<Material> mtrlFullscreenQuad;
		std::unique_ptr<VertexBuffer> m_vertexBuffer;
		Renderer* mRenderer;
		RenderTarget* rtDevice;
		RenderTarget* rtMain;
		RenderState* mRenderState;
		RefCountedPtr<Graphics::Texture> mRenderTexture;
		std::unique_ptr<Graphics::Drawables::TexturedQuad> mRenderQuad;
	};
}

#endif
