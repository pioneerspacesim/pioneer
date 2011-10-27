#include "RocketGaugeTypeBar.h"
#include "RocketGaugeElement.h"
#include "Rocket/Core.h"

RocketGaugeTypeBar::RocketGaugeTypeBar(RocketGaugeElement *el) : RocketGaugeType(el)
{
	orientation = HORIZONTAL;
	bar = 0;
	Initialize();
	//get attributes like orientation, direction
	//I guess direction could replace orientation...
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

	return dirty_layout;
}

void RocketGaugeTypeBar::ProcessEvent(Rocket::Core::Event& event)
{
	if (event == "resize" && event.GetTargetElement() == parent) {
		Rocket::Core::Vector2f box = parent->GetBox().GetSize();
		FormatElements(box, orientation == VERTICAL ? box.y : box.x);
	}
}

bool RocketGaugeTypeBar::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	if (orientation == HORIZONTAL) {
		dimensions.x = 100.f;
		dimensions.y = 50.f;
	} else {
		dimensions.x = 50.f;
		dimensions.y = 100.f;
	}
	return true;
}

void RocketGaugeTypeBar::FormatElements(const Rocket::Core::Vector2f& containingBlock, float length)
{
	float barLength = length;

	int lengthAxis = orientation == VERTICAL ? 1 : 0;
	Rocket::Core::Box parentBox;
	Rocket::Core::ElementUtilities::BuildBox(parentBox, Rocket::Core::Vector2f(containingBlock.x, containingBlock.y), parent);

	Rocket::Core::Vector2f content = parentBox.GetSize();
	content[lengthAxis] = barLength;
	parentBox.SetContent(content);

	/*bgLength -= (bgBox.GetCumulativeEdge(Rocket::Core::Box::CONTENT, Rocket::Core::Box::LEFT) +
				 bgBox.GetCumulativeEdge(Rocket::Core::Box::CONTENT, Rocket::Core::Box::RIGHT));*/

	// If no height has been explicitly specified for the bar, it'll be initialised to -1 as per normal block
	// elements. We'll fix that up here.
	//if orientation != vertical
	if (content.y < 0)
		content.y = parentBox.GetSize().y;

	//set margins - necessary?

	//format bar
	Rocket::Core::Box barBox;
	Rocket::Core::ElementUtilities::BuildBox(barBox, parent->GetBox().GetSize(), bar);
	
	Rocket::Core::Vector2f barBoxContent = barBox.GetSize();
	//if orientation == horizontal
	if (bar->GetLocalProperty("height") == 0)
		barBoxContent.y = parent->GetBox().GetSize().y;

	if (barLength >= 0)
	{
		Rocket::Core::Vector2f bgSize = parent->GetBox().GetSize();

		//if vertical
		//else
		{
			float background_length = bgSize.x -
				(barBox.GetCumulativeEdge(Rocket::Core::Box::CONTENT, Rocket::Core::Box::LEFT) +
				 barBox.GetCumulativeEdge(Rocket::Core::Box::CONTENT, Rocket::Core::Box::RIGHT));

			if (bar->GetLocalProperty("width") == 0)
			{

				//check for 'min-width' restrictions
				float min_bg_length = bar->ResolveProperty("min-width", background_length);
				barBoxContent.x = Rocket::Core::Math::Max(min_bg_length, barBoxContent.x);

				//check for 'max-width' restrictions'
				float max_bg_length = bar->ResolveProperty("max-width", background_length);
				if (max_bg_length > 0)
					barBoxContent.x = Rocket::Core::Math::Min(max_bg_length, barBoxContent.x);

			}

			barBoxContent.x = Rocket::Core::Math::Min(barBoxContent.x, background_length);
		}
	}

	// set the new dimensions on the bar to re-decorate it
	barBox.SetContent(barBoxContent);
	bar->SetBox(barBox);

	//position the resized bar
	const Rocket::Core::Vector2f& bgDimensions = parent->GetBox().GetSize();
	const Rocket::Core::Vector2f& barDimensions = bar->GetBox().GetSize(Rocket::Core::Box::BORDER);

	//if orientation == vertical
	//
	//else
	{
		bar->SetOffset(
			Rocket::Core::Vector2f(parent->GetRelativeOffset().x,
			bar->GetBox().GetEdge(Rocket::Core::Box::MARGIN, Rocket::Core::Box::TOP)), parent);
	}
}

