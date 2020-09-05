

#include "pigui/View.h"
#include "Pi.h"
#include "lua/LuaPiGui.h"

void PiGuiView::DrawPiGui()
{
	PiGUI::RunHandler(Pi::GetFrameTime(), m_handlerName);
}
