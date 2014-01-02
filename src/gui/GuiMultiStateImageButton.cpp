// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"

namespace Gui {
MultiStateImageButton::MultiStateImageButton(): Button()
{
	m_curState = 0;
	m_isSelected = true;
	Button::onClick.connect(sigc::mem_fun(this, &MultiStateImageButton::OnActivate));
}

MultiStateImageButton::~MultiStateImageButton()
{
	for (std::vector<State>::iterator i = m_states.begin(); i != m_states.end(); ++i) {
		delete (*i).activeImage;
		delete (*i).inactiveImage;
	}
}

void MultiStateImageButton::StateNext()
{
	m_curState++;
	if (m_curState >= signed(m_states.size())) m_curState = 0;
	UpdateOverriddenTooltip();
}

void MultiStateImageButton::StatePrev()
{
	m_curState--;
	if (m_curState < 0) m_curState = signed(m_states.size())-1;
	UpdateOverriddenTooltip();
}

void MultiStateImageButton::OnActivate()
{
	// only iterate through states once widget is selected.
	if (m_isSelected) StateNext();
	else {
		m_isSelected = true;
		onSelect.emit();
	}
	onClick.emit(this);
}

void MultiStateImageButton::SetActiveState(int state)
{
	for (unsigned int i=0; i<m_states.size(); i++) {
		if (m_states[i].state == state) {
			m_curState = i;
			break;
		}
	}
}

void MultiStateImageButton::SetSelected(bool state)
{
	m_isSelected = state;
}

void MultiStateImageButton::GetSizeRequested(float size[2])
{
	assert(m_states.size());
	m_states[0].activeImage->GetSizeRequested(size);
	m_states[0].inactiveImage->GetSizeRequested(size);
}

void MultiStateImageButton::Draw()
{
	PROFILE_SCOPED()
	float sz[2];
	GetSize(sz);
	if (m_isSelected) {
		m_states[m_curState].activeImage->SetSize(sz[0], sz[1]);
		m_states[m_curState].activeImage->Draw();
	} else {
		m_states[m_curState].inactiveImage->SetSize(sz[0], sz[1]);
		m_states[m_curState].inactiveImage->Draw();
	}
}

void MultiStateImageButton::AddState(int state, const char *filename)
{
	AddState(state, filename, filename, "");
}

void MultiStateImageButton::AddState(int state, const char *filename, std::string tooltip)
{
	AddState(state, filename, filename, tooltip);
}

void MultiStateImageButton::AddState(int state, const char *inactiveImage, const char *activeImage, std::string tooltip)
{
	State s;
	s.state = state;
	s.inactiveImage = new Image(inactiveImage);
	s.activeImage = new Image(activeImage);
	s.tooltip = tooltip;
	m_states.push_back(s);
	float size[2];
	s.activeImage->GetSizeRequested(size);
	SetSize(size[0], size[1]);
}

std::string MultiStateImageButton::GetOverrideTooltip()
{
	return m_states[m_curState].tooltip;
}

void MultiStateImageButton::SetRenderDimensions(const float wide, const float high)
{
	for (std::vector<State>::iterator i = m_states.begin(); i != m_states.end(); ++i) {
		assert((*i).activeImage && (*i).inactiveImage);
		(*i).activeImage->SetRenderDimensions(wide, high);
		(*i).inactiveImage->SetRenderDimensions(wide, high);
	}
}

}
