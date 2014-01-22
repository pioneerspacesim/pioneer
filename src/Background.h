// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Texture.h"
#include "Random.h"

namespace Graphics {
	class Renderer;
	class StaticMesh;
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
	};

	class UniverseBox : public BackgroundElement
	{
	public:
		UniverseBox(Graphics::Renderer *r);
		~UniverseBox();

		void Draw();
		void LoadCubeMap(Random &rand);

	private:
		void Init();

		std::unique_ptr<Graphics::StaticMesh> m_model;
		std::unique_ptr<Graphics::Texture> m_cubemap;

		Uint32 m_numCubemaps;
	};

	class Starfield : public BackgroundElement
	{
	public:
		//does not Fill the starfield
		Starfield(Graphics::Renderer *r, Random &rand);
		void Draw();
		//create or recreate the starfield
		void Fill(Random &rand);

	private:
		void Init();
		static const int BG_STAR_MAX = 10000;
		std::unique_ptr<Graphics::StaticMesh> m_model;

		//hyperspace animation vertex data
		vector3f m_hyperVtx[BG_STAR_MAX*2];
		Color m_hyperCol[BG_STAR_MAX*2];
	};

	class MilkyWay : public BackgroundElement
	{
	public:
		MilkyWay(Graphics::Renderer*);
		void Draw();

	private:
		std::unique_ptr<Graphics::StaticMesh> m_model;
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
		void Refresh(Random &rand);

		void SetIntensity(float intensity);
		void SetDrawFlags(const Uint32 flags);

	private:
		Graphics::Renderer *m_renderer;
		MilkyWay m_milkyWay;
		Starfield m_starField;
		UniverseBox m_universeBox;
		Uint32 m_drawFlags;
	};

} //namespace Background

#endif
