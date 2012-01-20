#ifndef _RENDERER_LEGACY_H
#define _RENDERER_LEGACY_H

#include "Renderer.h"

// Fixed function renderer
class RendererLegacy : public Renderer
{
public:
	RendererLegacy(int width, int height);
	virtual ~RendererLegacy();

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool SetBlendMode(unsigned int blendType);

	virtual bool DrawLines(int vertCount, const LineVertex *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const LineVertex2D *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, unsigned int type=TRIANGLES);
	virtual bool DrawTriangles2D(const VertexArray *vertices, const Material *material=0, unsigned int type=TRIANGLES);
	virtual bool DrawSurface2D(const Surface *surface);
};

#endif