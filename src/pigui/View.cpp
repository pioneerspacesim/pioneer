// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "pigui/View.h"
#include "LuaPiGui.h"
#include "Pi.h"

void PiGuiView::DrawPiGui()
{
	PiGui::RunHandler(Pi::GetFrameTime(), m_handlerName);
}
