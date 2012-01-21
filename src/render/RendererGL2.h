#include "render/Renderer.h"
#include "render/RendererLegacy.h"

/*
 * OpenGL 2.x renderer
 *  - no fixed function support (shaders for everything)
 *  - try to stick to bufferobjects
 *  - avoid glVertexPointer, glColorPointer?
 */
class RendererGL2 : public RendererLegacy // XXX not really desired, just want to get up to speed
{
public:
	RendererGL2(int width, int height);
	virtual ~RendererGL2();
};