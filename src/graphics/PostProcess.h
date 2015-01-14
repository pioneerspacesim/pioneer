// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _POST_PROCESS_H_
#define _POST_PROCESS_H_

#include "libs.h"
#include "RenderTarget.h"
#include "Material.h"

namespace Graphics {
	class WindowSDL;
	class Material;
	class Renderer;
	class PostProcessing;

	enum PostProcessPassType
	{
		PP_PASS_THROUGH, // Normal pass: last pass as input, new pass as output
		PP_PASS_COMPOSE, // Composition pass: main rt and last pass as input, new pass as output
	};

	struct PostProcessPass
	{
		std::string name;
		std::shared_ptr<Material> material;
		std::unique_ptr<RenderTarget> renderTarget;
		PostProcessPassType type;
	};

	class PostProcess
	{
		friend class PostProcessing;
	public:
		// Constructs a custom render target
		PostProcess(const std::string& effect_name, RenderTargetDesc& rtd);
		// Constructs a color render target that matches the display mode
		PostProcess(const std::string& effect_name, WindowSDL* window);
		virtual ~PostProcess();

		void AddPass(Renderer* renderer, const std::string& pass_name, std::shared_ptr<Material>& material, PostProcessPassType pass_type = PP_PASS_THROUGH);
		void AddPass(Renderer* renderer, const std::string& pass_name, Graphics::EffectType effect_type, PostProcessPassType pass_type = PP_PASS_THROUGH);

		// Accessors
		unsigned int GetPassCount() const { return vPasses.size(); }

	protected:		

	private:
		PostProcess(const PostProcess&);
		PostProcess& operator=(const PostProcess&);

		std::string strName;
		std::vector<PostProcessPass*> vPasses;
		std::unique_ptr<RenderTargetDesc> mRTDesc;
	};
}

#endif
