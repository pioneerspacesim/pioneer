// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DRAWABLES_H
#define _DRAWABLES_H

#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "libs.h"

namespace Graphics {
	class Renderer;

	namespace Drawables {

		// A thing that can draw itself using renderer
		// (circles, disks, polylines etc)
		//------------------------------------------------------------

		// Two-dimensional open circle.
		/* TODO: reimplement as an immediate-mode API writing to a shared vertex array for debug use
		class Circle {
		public:
			Circle(Renderer *renderer, const float radius, const Color &c, RenderState *state);
			Circle(Renderer *renderer, const float radius, const float x, const float y, const float z, const Color &c, RenderState *state);
			Circle(Renderer *renderer, const float radius, const vector3f &center, const Color &c, RenderState *state);
			void Draw(Renderer *renderer);

		private:
			void SetupVertexBuffer(const Graphics::VertexArray &, Graphics::Renderer *);
			RefCountedPtr<VertexBuffer> m_vertexBuffer;
			RefCountedPtr<Material> m_material;
			Color m_color;
			Graphics::RenderState *m_renderState;
		};
		*/
		//------------------------------------------------------------

		// Two-dimensional filled circle
		// Generates a TRIANGLE_FAN primitive
		class Disk {
		public:
			Disk(Renderer *r, const int edges = 72, const float radius = 1.0f);
			void Draw(Renderer *r, Material *mat);

		private:
			std::unique_ptr<MeshObject> m_diskMesh;
		};
		//------------------------------------------------------------

		// A three dimensional line between two points
		/* TODO: reimplement as an immediate-mode API writing to a shared vertex array for debug use
		class Line3D {
		public:
			Line3D();
			Line3D(const Line3D &b); // this needs an explicit copy constructor due to the std::unique_ptr below
			~Line3D() {}
			void SetStart(const vector3f &);
			void SetEnd(const vector3f &);
			void SetColor(const Color &);
			void Draw(Renderer *, RenderState *);

		private:
			void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);
			void Dirty();

			bool m_refreshVertexBuffer;
			float m_width;
			RefCountedPtr<Material> m_material;
			RefCountedPtr<VertexBuffer> m_vertexBuffer;
			std::unique_ptr<Graphics::VertexArray> m_va;
		};
		*/

		//------------------------------------------------------------

		// Three dimensional line segments between two points
		// Data can be drawn with any of the LINE_* primitive types depending on what the calling code intends
		class Lines {
		public:
			Lines();
			void SetData(const Uint32 vertCount, const vector3f *vertices, const Color &color);
			void SetData(const Uint32 vertCount, const vector3f *vertices, const Color *colors);
			void Draw(Renderer *, Material *);

		private:
			bool m_refreshVertexBuffer;
			RefCountedPtr<MeshObject> m_lineMesh;
			std::unique_ptr<VertexArray> m_va;
		};
		//------------------------------------------------------------

		// Screen aligned quad / billboard / pointsprite
		// Material must be created with a primitive type of Graphics::POINTS
		class PointSprites {
		public:
			PointSprites();
			void SetData(const int count, const vector3f *positions, const Color *colours, const float *sizes);
			void Draw(Renderer *, Material *);

		private:
			bool m_refreshVertexBuffer;
			RefCountedPtr<Graphics::MeshObject> m_pointData;
			std::unique_ptr<VertexArray> m_va;
		};
		//------------------------------------------------------------

		// Screen aligned quad / billboard / pointsprite
		class Points {
		public:
			Points();
			void SetData(Renderer *, const int count, const vector3f *positions, const matrix4x4f &trans, const Color &color, const float size);
			void SetData(Renderer *, const int count, const vector3f *positions, const Color *color, const matrix4x4f &trans, const float size);
			void Draw(Renderer *, Material *);

		private:
			void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);

			bool m_refreshVertexBuffer;
			RefCountedPtr<MeshObject> m_pointMesh;
			std::unique_ptr<VertexArray> m_va;
		};
		//------------------------------------------------------------

		// Three dimensional sphere (subdivided icosahedron) with normals
		// and spherical texture coordinates.
		class Sphere3D {
		public:
			//subdivisions must be 0-4
			Sphere3D(Renderer *, RefCountedPtr<Material> material, int subdivisions = 0, float scale = 1.f, AttributeSet attribs = (ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0));
			void Draw(Renderer *r);

			RefCountedPtr<Material> GetMaterial() const { return m_material; }

		private:
			std::unique_ptr<MeshObject> m_sphereMesh;
			RefCountedPtr<Material> m_material;

			//std::unique_ptr<Surface> m_surface;
			//add a new vertex, return the index
			int AddVertex(VertexArray &, const vector3f &v, const vector3f &n);
			//add three vertex indices to form a triangle
			void AddTriangle(std::vector<Uint32> &, int i1, int i2, int i3);
			void Subdivide(VertexArray &, std::vector<Uint32> &,
				const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
				int i1, int i2, int i3, int depth);
		};
		//------------------------------------------------------------

		// a textured quad with reversed winding
		/* TODO: reimplement this as an immediate-mode API for debug use
		class TexturedQuad {
		public:
			// Simple constructor to build a textured quad from an image.
			// Note: this is intended for UI icons and similar things, and it builds the
			// texture with that in mind (e.g., no texture compression because compression
			// tends to create visible artefacts when used on UI-style textures that have
			// edges/lines, etc)
			// XXX: This is totally the wrong place for this helper function.
			TexturedQuad(Graphics::Renderer *r, const std::string &filename);

			// Build a textured quad to display an arbitrary texture.
			TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size, RenderState *state);
			TexturedQuad(Graphics::Renderer *r, RefCountedPtr<Graphics::Material> &material, const Graphics::VertexArray &va, RenderState *state);

			void Draw(Graphics::Renderer *r);
			void Draw(Graphics::Renderer *r, const Color4ub &tint);
			const Graphics::Texture *GetTexture() const { return m_texture.Get(); }

		private:
			RefCountedPtr<Graphics::Texture> m_texture;
			RefCountedPtr<Graphics::Material> m_material;
			RefCountedPtr<VertexBuffer> m_vertexBuffer;
			Graphics::RenderState *m_renderState;
		};
		*/

		//------------------------------------------------------------

		// a coloured rectangle
		/* TODO: reimplement this as an immediate-mode API for debug use
		class Rect {
		public:
			Rect(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const Color &c, RenderState *state, const bool bIsStatic = true);
			void Update(const vector2f &pos, const vector2f &size, const Color &c);
			void Draw(Graphics::Renderer *r);

		private:
			RefCountedPtr<Graphics::Material> m_material;
			RefCountedPtr<VertexBuffer> m_vertexBuffer;
			Graphics::RenderState *m_renderState;
		};
		*/

		//------------------------------------------------------------

		// a coloured rectangle
		/* TODO: reimplement this as an immediate-mode API for debug use
		class RoundEdgedRect {
		public:
			RoundEdgedRect(Graphics::Renderer *r, const vector2f &size, const float rad, const Color &c, RenderState *state, const bool bIsStatic = true);
			void Update(const vector2f &size, float rad, const Color &c);
			void Draw(Graphics::Renderer *r);

		private:
			static const int STEPS = 6;
			RefCountedPtr<Graphics::Material> m_material;
			RefCountedPtr<VertexBuffer> m_vertexBuffer;
			Graphics::RenderState *m_renderState;
		};
		*/

		//------------------------------------------------------------

		//industry-standard red/green/blue XYZ axis indicator
		class Axes3D {
		public:
			Axes3D(Graphics::Renderer *r);
			void Draw(Graphics::Renderer *r);

		private:
			RefCountedPtr<Graphics::MeshObject> m_axesMesh;
			RefCountedPtr<Graphics::Material> m_axesMat;
		};

		Axes3D *GetAxes3DDrawable(Graphics::Renderer *r);

	} // namespace Drawables

} // namespace Graphics

#endif
