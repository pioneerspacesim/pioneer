// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "graphics/Texture.h"
#include "graphics/RenderState.h"
#include "Random.h"

namespace Graphics {
	class Renderer;
	class Material;
}

/*
 * Classes to draw background stars and the milky way
 */

namespace Background
{
	class BackgroundElement
	{
	public:
		void SetIntensity(float intensity);

	protected:
		Graphics::Renderer *m_renderer;
		RefCountedPtr<Graphics::Material> m_material;
		RefCountedPtr<Graphics::Material> m_materialStreaks;

		float m_rMin;
		float m_rMax;
		float m_gMin;
		float m_gMax;
		float m_bMin;
		float m_bMax;
	};

	class UniverseBox : public BackgroundElement
	{
	public:
		UniverseBox(Graphics::Renderer *r);
		~UniverseBox();

		void Draw(Graphics::RenderState*);
		void LoadCubeMap(Random &rand);

	private:
		void Init();

		std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
		RefCountedPtr<Graphics::Texture> m_cubemap;

		Uint32 m_numCubemaps;
	};

	class Starfield : public BackgroundElement
	{
	public:
		//does not Fill the starfield
		Starfield(Graphics::Renderer *r, Random &rand);
		void Draw(Graphics::RenderState*);
		//create or recreate the starfield
		void Fill(Random &rand);

	private:
		void Init();

		std::unique_ptr<Graphics::Drawables::PointSprites> m_pointSprites;
		Graphics::RenderState* m_renderState; // NB: we don't own RenderState pointers, just borrow them

		//hyperspace animation vertex data
		std::unique_ptr<vector3f[]> m_hyperVtx;// BG_STAR_MAX * 3
		std::unique_ptr<Color[]> m_hyperCol;// BG_STAR_MAX * 3
		std::unique_ptr<Graphics::VertexBuffer> m_animBuffer;
	};

	class MilkyWay : public BackgroundElement
	{
	public:
		MilkyWay(Graphics::Renderer*);
		void Draw(Graphics::RenderState*);

	private:
		std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
	};



	// contains starfield, milkyway, possibly other Background elements
	class Container
	{
	public:
		enum BackgroundDrawFlags
		{
			DRAW_STARS = 1<<0,
			DRAW_MILKY = 1<<1,
			DRAW_SKYBOX = 1<<2
		};

		Container(Graphics::Renderer*, Random &rand);
		void Draw(const matrix4x4d &transform);

		void SetIntensity(float intensity);
		void SetDrawFlags(const Uint32 flags);

	private:
		void Refresh(Random &rand);

		Graphics::Renderer *m_renderer;
		MilkyWay m_milkyWay;
		Starfield m_starField;
		UniverseBox m_universeBox;
		Uint32 m_drawFlags;
		Graphics::RenderState *m_renderState;
	};

} //namespace Background

#endif
