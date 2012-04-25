#include "LOD.h"

namespace Newmodel {

LOD::LOD() : Group()
{
}

void LOD::Render(Graphics::Renderer *renderer, const matrix4x4f &trans)
{
	//figure out approximate pixel size on screen and pick a child to render
	//float pixrad = 0.5f * s_scrWidth * rstate->combinedScale * m_drawClipRadius / cameraPos.Length();
	//int lod = m_numLods-1;
	//for (int i=lod-1; i>=0; i--) {
	//	if (pixrad < m_lodPixelSize[i]) lod = i;
	//}
}

}