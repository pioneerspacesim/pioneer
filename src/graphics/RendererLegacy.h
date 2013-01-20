// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_LEGACY_H
#define _RENDERER_LEGACY_H
/*
 * Fixed function renderer (GL1.5 approx)
 */
#include "Renderer.h"

namespace Graphics {

class Texture;
struct Settings;

class RendererLegacy : public Renderer
{
public:
	RendererLegacy(const Graphics::Settings &vs);
	virtual ~RendererLegacy();

	virtual const char* GetName() const { return "Legacy renderer"; }
	virtual bool GetNearFarRange(float &near, float &far) const;

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool ClearScreen();
	virtual bool ClearDepthBuffer();
	virtual bool SetClearColor(const Color &c);

	virtual bool SetViewport(int x, int y, int width, int height);

	virtual bool SetTransform(const matrix4x4d &m);
	virtual bool SetTransform(const matrix4x4f &m);
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far);
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

	virtual bool SetBlendMode(BlendMode mode);
	virtual bool SetDepthTest(bool enabled);
	virtual bool SetDepthWrite(bool enabled);
	virtual bool SetWireFrameMode(bool enabled);

	virtual bool SetLights(int numlights, const Light *l);
	virtual bool SetAmbientColor(const Color &c);

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f));

	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, LineType type=LINE_SINGLE);
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, LineType type=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const vector2f *vertices, const Color &color, LineType type=LINE_SINGLE);
	virtual bool DrawPoints(int count, const vector3f *points, const Color *colors, float pointSize=1.f);
	virtual bool DrawPoints2D(int count, const vector2f *points, const Color *colors, float pointSize=1.f);
	virtual bool DrawTriangles(const VertexArray *vertices, Material *material, PrimitiveType type=TRIANGLES);
	virtual bool DrawSurface(const Surface *surface);
	virtual bool DrawPointSprites(int count, const vector3f *positions, Material *material, float size);
	virtual bool DrawStaticMesh(StaticMesh *thing);

	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor);
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor);

	virtual bool PrintDebugInfo(std::ostream &out);

protected:
	virtual void PushState();
	virtual void PopState();

	//figure out states from a vertex array and enable them
	//also sets vertex pointers
	virtual void EnableClientStates(const VertexArray *v);
	//disable previously enabled
	virtual void DisableClientStates();
	int m_numLights;
	int m_numDirLights;
	std::vector<GLenum> m_clientStates;
	virtual bool BufferStaticMesh(StaticMesh *m);
	float m_minZNear;
	float m_maxZFar;
	bool m_useCompressedTextures;

	matrix4x4f& GetCurrentTransform() { return m_currentTransform; }
	matrix4x4f m_currentTransform;
};

}

#endif
