#include "RocketGaugeTypeBar.h"
#include "RocketGaugeElement.h"
#include "Rocket/Core.h"

RocketGaugeTypeBar::RocketGaugeTypeBar(RocketGaugeElement *el) : RocketGaugeType(el)
{
	direction = RIGHT;
	bar = 0;
	Initialize();
}

RocketGaugeTypeBar::~RocketGaugeTypeBar()
{
	if (bar != 0)
		parent->RemoveChild(bar);
}

bool RocketGaugeTypeBar::Initialize()
{
	bar = Rocket::Core::Factory::InstanceElement(parent, "*", "gaugebar", Rocket::Core::XMLAttributes());

	if (bar == 0)
		return false;

	//non-DOM add (hidden)
	parent->AppendChild(bar, false);

	//remove initial reference
	bar->RemoveReference();

	//no event listeners. Should probably support focus & blur

	return true;
}

void RocketGaugeTypeBar::OnUpdate()
{
}

void RocketGaugeTypeBar::OnRender()
{
}

bool RocketGaugeTypeBar::OnAttributeChange(const Rocket::Core::AttributeNameList& changedAttributes)
{
	bool dirty_layout = false;

	//has orientation changed? update & dirty
	if (changedAttributes.find("direction") != changedAttributes.end()) {
		Rocket::Core::String dirstr = parent->GetAttribute< Rocket::Core::String >("direction", "right");
		Direction newdir = RIGHT;
		if (dirstr == "left")
			newdir = LEFT;
		else if (dirstr == "up")
			newdir = UP;
		else if (dirstr == "down")
			newdir = DOWN;
		if (direction != newdir) {
			direction = newdir;
			dirty_layout = true;
		}
	}

	return dirty_layout;
}

void RocketGaugeTypeBar::ProcessEvent(Rocket::Core::Event& event)
{
	//resize is called on initial layout as well
	if (event == "resize" && event.GetTargetElement() == parent) {
		Rocket::Core::Vector2f box = parent->GetBox().GetSize();
		float w = box.y;
		if (direction == UP || direction == DOWN)
			w = box.x;
		FormatElements(box, direction == RIGHT ? box.y : box.x);
	}
}

bool RocketGaugeTypeBar::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	if (direction == RIGHT || direction == LEFT) {
		dimensions.x = 256.f;
		dimensions.y = 16.f;
	} else {
		dimensions.x = 16.f;
		dimensions.y = 256.f;
	}
	return true;
}

void RocketGaugeTypeBar::FormatElements(const Rocket::Core::Vector2f& containingBlock, float length)
{
	bool horizontal = (direction == RIGHT || direction == LEFT);
	Rocket::Core::Box barBox;
	Rocket::Core::ElementUtilities::BuildBox(barBox, parent->GetBox().GetSize(), bar);

	Rocket::Core::Vector2f barBoxContent = barBox.GetSize();

	// If no height is specified, it would be zero
	if (bar->GetLocalProperty("height") == 0)
		barBoxContent.y = parent->GetBox().GetSize().y;

	//calculate min-width/height limitations
	//calculate margin limitations
	//calculate padding
	//calculate border
	if (horizontal) {
		barBoxContent.x *= parent->GetGaugeValue();
	} else {
		barBoxContent.y *= parent->GetGaugeValue();
	}

	//set updated width
	barBox.SetContent(barBoxContent);
	bar->SetBox(barBox);

	//Position the bar
	//this is *not* correct wrt bar margins & background padding
	switch (direction) {
	case RIGHT:
		bar->SetOffset(Rocket::Core::Vector2f(0.f, 0.f), parent);
		break;
	case LEFT:
		bar->SetOffset(Rocket::Core::Vector2f(
			parent->GetBox().GetSize().x - barBoxContent.x,
			0.f), parent);
		break;
	case UP:
		bar->SetOffset(Rocket::Core::Vector2f(
			0.f,
			parent->GetBox().GetSize().y - barBoxContent.y),
			parent);
		break;
	case DOWN:
		bar->SetOffset(Rocket::Core::Vector2f(0.f, 0.f), parent);
		break;
	}
}

