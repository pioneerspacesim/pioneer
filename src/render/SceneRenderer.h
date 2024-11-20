// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

/**
 * This file implements a high-level data-driven renderer.
 *
 * Its main concern is to render "scene" objects while providing a flexible
 * interface to manage post-processing effects, render techniques, and being
 * able to adapt to significant differences in device performance by providing
 * a built-in mechanism to control which rendering effects are executed.
 *
 * To that end, the SceneRenderer's main concern is to apply the RenderPasses
 * defined in a RenderSetup file to the objects in a scene.
 *
 * Many rendering operations are implemented as composable RenderGenerators,
 * allowing a data file to control the entire render pipe with a minimum of
 * code needing to change.
 *
 * This file is intentionally incomplete. Many planned features are not yet
 * implemented, as doing so now would introduce an unnecessarily tight coupling
 * to the Graphics::Renderer and SceneGraph APIs, which need to undergo
 * significant changes themselves.
 *
 * A short high-level TODO list follows:
 *
 * - TODO: implement conditional execution of RenderPasses
 * - TODO: split Graphics::RenderTarget into color/depth textures which can be
 *         bound together at draw time into a framebuffer object
 * - TODO: support more than one color target per render pass
 * - TODO: add a mechanism to pass additional parameters into a shader from a
 *         RenderPass section inside a RenderSetup file
 * - TODO: split responsibility for rendering lit objects into a separate
 *         Generator
 * - TODO: implement RenderObject(s) and RenderScene abstraction to separate
 *         rendering-related resources from "game code"
 * - TODO: migrate Body to create and hold a handle to RenderObject(s) rather
 *         than being responsible for directly submitting graphics API commands
 * - TODO: walk the list of active RenderObjects once and bin to individual
 *         passes rather than each pass doing an ad-hoc walk of the entire list
 */

#pragma once

#include "Camera.h"
#include "Color.h"
#include "matrix4x4.h"
#include "vector3.h"

#include "graphics/Light.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Body;
class CameraContext;
class Frame;
class Space;
class SpaceStation;

namespace Graphics {
	class Material;
	class Renderer;
	class RenderTarget;
}

namespace Render {

	// Forward declarations
	struct RenderPass;

	class RenderSetup;
	class ResourceManager;
	class SceneRenderer;

	/**
	 * RenderGenerator is responsible for implementing a single render pass
	 * technique.
	 *
	 * It's invoked to execute a specific RenderPass as controlled by a
	 * RenderSetup file and will have its output render targets already setup
	 * when invoked.
	 */
	class RenderGenerator {
	public:
		RenderGenerator(SceneRenderer *r) :
			m_sceneRenderer(r)
		{}
		virtual ~RenderGenerator() {}

		virtual void CacheShadersForPass(RenderPass *pass);

		virtual void Run(RenderPass *pass, Space *space, const CameraContext *camera) = 0;

	protected:
		SceneRenderer *m_sceneRenderer;
	};

	/**
	 * SceneRenderer is responsible for managing the high-level rendering
	 * process as defined in the RenderSetup file.
	 *
	 * The main thread should call BeforeRendering() once, and then call
	 * RenderScene(..., "layer") as many times as needed.
	 *
	 * Each layer is defined in the RenderSetup file, composed of a list of
	 * RenderPasses with associated RenderGenerators. Each RenderPass can
	 * optionally specify a specific shader it would like to use when executing
	 * (if supported by the selected RenderGenerator).
	 *
	 * At the moment, the SceneRenderer also assumes responsibility for the
	 * LitObjects "main pass" rendering, but this should be split to a separate
	 * Generator once object binning is implemented.
	 */
	class SceneRenderer {
	public:
		struct Light {
			Graphics::Light light;
			Body *body;
		};

		struct BodyAttrs {
			Body *body;
			vector3d viewCoords;
			double camDist;
			matrix4x4d viewTransform;

			float billboardSize;
			Color billboardColor;

			uint32_t exclude : 1;
			uint32_t calcAtmosphericLighting : 1;
			uint32_t calcInteriorLighting : 1;
			uint32_t billboard : 1;
			uint32_t bodyFlags;
		};

		SceneRenderer(Graphics::Renderer *r, ResourceManager *rm);
		~SceneRenderer();

		void LoadRenderSetup(RenderSetup *setup);

		// Prep all needed datastructures to render the scene
		void BeforeRendering(Space *space, Camera *camera);

		// Render the scene with the given render layer
		void RenderScene(Space *space, Camera *camera, std::string_view renderLayer);

		Graphics::Renderer *GetRenderer() { return m_renderer; }
		RenderSetup *GetRenderSetup() { return m_renderSetup.get(); }
		ResourceManager *GetResourceManager() { return m_resManager; }

		Graphics::Material *GetShaderForPass(RenderPass *pass, std::string_view shaderName);
		void CacheShaderForPass(RenderPass *pass, std::string_view shaderName, Graphics::Material *mat);

		std::vector<Light> &GetLights() { return m_lightSources; }
		std::vector<BodyAttrs> &GetSortedBodies() { return m_sortedBodies; }

	private:
		struct CachedMaterial {
			CachedMaterial(std::string_view n, Graphics::Material *m) :
				name(n),
				mat(m)
			{}

			std::string name;
			std::unique_ptr<Graphics::Material> mat;
		};

		void BuildSortedBodies(Space *space, const CameraContext *camera);
		void SetupRenderPass(RenderPass *pass);

		void RenderLitObjects(RenderPass *pass, Space *space, Camera *camera);
		void PrepareLighting(BodyAttrs *attrs);

		Graphics::Renderer *m_renderer;
		ResourceManager *m_resManager;

		std::unique_ptr<RenderSetup> m_renderSetup;

		std::vector<Camera::LightSource> m_camLights;
		std::vector<SpaceStation *> m_spaceStations;

		std::vector<Light> m_lightSources;
		std::vector<BodyAttrs> m_sortedBodies;

		std::map<std::string, std::unique_ptr<RenderGenerator>> m_generators;

		// NOTE: we use a simple vector of structs rather than another map
		// because the N of materials per pass is unlikely to exceed the point where
		// brute-force vector lookup becomes slower than a map lookup.
		std::map<RenderPass *, std::vector<CachedMaterial>> m_passMaterials;
	};

} // namespace Render
