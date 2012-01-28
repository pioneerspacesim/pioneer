#ifndef _RENDERER_LEGACY_H
#define _RENDERER_LEGACY_H

#include "Renderer.h"

// Fixed function renderer
class RendererLegacy : public Renderer
{
public:
	RendererLegacy(int width, int height);
	virtual ~RendererLegacy();

	virtual const char* GetName() { return "Legacy renderer"; }

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool SetBlendMode(BlendMode mode);

	virtual bool SetLights(int numlights, const Light *l);
	virtual bool SetAmbientColor(const Color &c);

	virtual bool DrawLines(int vertCount, const LineVertex *vertices, LineType lineType=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const vector2f *vertices, const Color &color, LineType type=LINE_SINGLE);
	virtual bool DrawPoints(int count, const vector3f *points, const Color *colors, float pointSize=1.f);
	virtual bool DrawPoints2D(int count, const vector2f *points, const Color *colors, float pointSize=1.f);
	virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES);
	virtual bool DrawTriangles2D(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES);
	virtual bool DrawSurface2D(const Surface *surface);
	virtual bool DrawPointSprites(int count, const vector3f *positions, const Material *material, float size);
	virtual bool DrawStaticMesh(StaticMesh *thing);

protected:
	int m_numLights;
};

#endif