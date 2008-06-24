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
		void AddState(int state, const char *filename);
		int GetState() { return m_states[m_curState].state; }
		void StateNext();
		void StatePrev();
		virtual void OnActivate();
		sigc::signal<void, MultiStateImageButton*> onClick;
		virtual void SetSelected(bool state);
	private:
		struct State {
			int state;
			Image *image;
		};
		std::vector<State> m_states;
		int m_curState;
		bool m_isSelected;
	};
}

#endif /* _GUIMULTISTATEIMAGEBUTTON_H */
