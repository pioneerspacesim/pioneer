#include "RocketGaugeTypeBar.h"
#include "RocketGaugeElement.h"
#include "Rocket/Core.h"

RocketGaugeTypeBar::RocketGaugeTypeBar(RocketGaugeElement *el) : RocketGaugeType(el)
{
	m_direction = RIGHT;
	m_bar = 0;
	Initialize();
}

RocketGaugeTypeBar::~RocketGaugeTypeBar()
{
	if (m_bar != 0)
		m_parent->RemoveChild(m_bar);
}

bool RocketGaugeTypeBar::Initialize()
{
	m_bar = Rocket::Core::Factory::InstanceElement(m_parent, "*", "gaugebar", Rocket::Core::XMLAttributes());

	if (m_bar == 0)
		return false;

	//non-DOM add (hidden)
	m_parent->AppendChild(m_bar, false);

	//remove initial reference
	m_bar->RemoveReference();

	//no event listeners. Should probably support focus & blur

	return true;
}

void RocketGaugeTypeBar::OnUpdate()
{
}

void RocketGaugeTypeBar::OnRender()
{
}

bool RocketGaugeTypeBar::OnAttributeChange(const Rocket::Core::AttributeNameList &changedAttributes)
{
	bool dirtyLayout = false;

	//has orientation changed? update & dirty
	if (changedAttributes.find("direction") != changedAttributes.end()) {
		Rocket::Core::String dirstr = m_parent->GetAttribute< Rocket::Core::String >("direction", "right");
		Direction newdir = RIGHT;
		if (dirstr == "left")
			newdir = LEFT;
		else if (dirstr == "up")
			newdir = UP;
		else if (dirstr == "down")
			newdir = DOWN;
		if (m_direction != newdir) {
			m_direction = newdir;
			dirtyLayout = true;
		}
	}

	return dirtyLayout;
}

void RocketGaugeTypeBar::ProcessEvent(Rocket::Core::Event &event)
{
	//resize is called on initial layout as well
	if (event == "resize" && event.GetTargetElement() == m_parent)
		FormatElements();
}

bool RocketGaugeTypeBar::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	if (m_direction == RIGHT || m_direction == LEFT) {
		dimensions.x = 256.f;
		dimensions.y = 16.f;
	} else {
		dimensions.x = 16.f;
		dimensions.y = 256.f;
	}
	return true;
}

void RocketGaugeTypeBar::FormatElements()
{
	const bool horizontal = (m_direction == RIGHT || m_direction == LEFT);
	Rocket::Core::Box barBox;

	//might not be a good idea to rebuild the box every time
	Rocket::Core::ElementUtilities::BuildBox(barBox, m_parent->GetBox().GetSize(), m_bar);

	Rocket::Core::Vector2f barBoxContent = barBox.GetSize();

	// If no height is specified, it would be zero
	if (m_bar->GetLocalProperty("height") == 0)
		barBoxContent.y = m_parent->GetBox().GetSize().y;

	if (horizontal) {
		barBoxContent.x *= m_parent->GetGaugeValue();
	} else {
		barBoxContent.y *= m_parent->GetGaugeValue();
	}

	//set updated width
	barBox.SetContent(barBoxContent);
	m_bar->SetBox(barBox);

	//Position the bar
	//this takes borders into account but not padding
	const float leftEdge = m_parent->GetBox().GetEdge(Rocket::Core::Box::BORDER, Rocket::Core::Box::LEFT);
	const float topEdge = m_parent->GetBox().GetEdge(Rocket::Core::Box::BORDER, Rocket::Core::Box::TOP);

	switch (m_direction) {
	case RIGHT:
		m_bar->SetOffset(Rocket::Core::Vector2f(
			leftEdge,
			topEdge), m_parent);
		break;
	case LEFT:
		m_bar->SetOffset(Rocket::Core::Vector2f(
			leftEdge + m_parent->GetBox().GetSize().x - barBoxContent.x,
			topEdge), m_parent);
		break;
	case UP:
		m_bar->SetOffset(Rocket::Core::Vector2f(
			leftEdge,
			topEdge + m_parent->GetBox().GetSize().y - barBoxContent.y),
			m_parent);
		break;
	case DOWN:
		m_bar->SetOffset(Rocket::Core::Vector2f(
			leftEdge,
			topEdge), m_parent);
		break;
	}
}

void RocketGaugeTypeBar::OnValueChanged()
{
	FormatElements();
}