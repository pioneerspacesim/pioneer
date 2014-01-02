// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIMULTISTATEIMAGEBUTTON_H
#define _GUIMULTISTATEIMAGEBUTTON_H

#include "GuiButton.h"
#include "GuiISelectable.h"
#include <string>
#include <vector>

namespace Gui {
	class MultiStateImageButton: public Button, public ISelectable {
	public:
		MultiStateImageButton();
		virtual void Draw();
		virtual ~MultiStateImageButton();
		virtual void GetSizeRequested(float size[2]);
		void AddState(int state, const char *icon);
		void AddState(int state, const char *icon, std::string tooltip);
		void AddState(int state, const char *inactiveIcon, const char *activeIcon, std::string tooltip);
		int GetState() { return m_states[m_curState].state; }
		void StateNext();
		void StatePrev();
		virtual void OnActivate();
		sigc::signal<void, MultiStateImageButton*> onClick;
		virtual void SetSelected(bool state);
		void SetActiveState(int state);
		void SetRenderDimensions(const float wide, const float high);
	protected:
		virtual std::string GetOverrideTooltip();
	private:
		struct State {
			int state;
			Image *inactiveImage;
			Image *activeImage;
			std::string tooltip;
		};
		std::vector<State> m_states;
		int m_curState;
		bool m_isSelected;
	};
}

#endif /* _GUIMULTISTATEIMAGEBUTTON_H */
