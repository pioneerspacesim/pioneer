local ui = Engine.ui

--
-- Class: uilib
--
-- Handy wrapper functions for UI elements.
--
-- Group: Functions
--

uilib = { -- Suggestions for a better namespace gladly accepted

--
-- Function: FaceWidget
--
-- Builds and returns a UI widget containing a face and a superimposed caption
-- from a <Character> object.
--
--   faceWidget = uilib.FaceWidget(character)
--
-- Return:
--
--   facewidget - a UI widget with a face and a caption
--
-- Parameters:
--
--   character - a <Character> object
--
-- Example:
--
-- > client = Character.New({title='Entrepreneur'}) -- random person with specified title
-- > form:PackEnd(uilib.FaceWidget(client))         -- Their picture, with name and
-- >                                                -- "Entrepreneur" on a caption
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- experimental
--
	FaceWidget = function (character)
		if not (character and (type(character)=='table') and (getmetatable(character).class == 'Character'))
		then
			error ('FaceWidget: Character object expected.')
		end
		local faceFlags = {
			character.female and "FEMALE" or "MALE",
			character.armor and "ARMOUR",
		}
		return UI.Game.Face.New(ui, faceFlags, character.seed)
		-- face + name gradient
		:SetInnerWidget(
			ui:Align("BOTTOM_LEFT"):SetInnerWidget(
				ui:Expand("HORIZONTAL"):SetInnerWidget(
					ui:Gradient({r=0.1,g=0.1,b=0.1,a=0.8}, {r=0.0,g=0.0,b=0.1,a=0.0}, "HORIZONTAL"):SetInnerWidget(
						ui:Margin(20):SetInnerWidget(ui:VBox():PackEnd({
							ui:Label(character.name):SetFont("HEADING_NORMAL"),
							ui:Label(character.title or ''):SetFont("HEADING_SMALL"),
						}))
					)
				)
			)
		)
	end,

--
-- Function: UpdateFaceText
--
-- Rebuilds the UI widget tree of a face widget, as returned by <FaceWidget>,
-- replacing the name and title in teh caption with that of the supplied
-- <Character> object.
--
--   uilib.UpdateFaceText(widget,character)
--
-- Parameters:
--
--   widget - a UI element previously returned by <FaceWidget>
--   character - a <Character> object
--
-- Example:
--
-- > client = Character.New({title='Entrepreneur'}) -- random person with specified title
-- > clientWidget = uilib.FaceWidget(client)        -- keep a reference to the widget
-- > form:PackEnd(clientWidget)                     -- display the widget
--
-- > client.title = "Beggar"                        -- Our client hits hard times
-- > uilib.UpdateFaceText(clientWidget,client)      -- Update the image on screen
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- experimental
--
	UpdateFaceText = function (widget,character)
		if not (widget and (type(widget) == 'userdata'))
		then
			error('UpdateFaceText: Widget expected.')
		end
		if not (character and (type(character)=='table') and (getmetatable(character).class == 'Character'))
		then
			error ('UpdateFaceText: Character object expected.')
		end
		widget:SetInnerWidget(
			ui:Align("BOTTOM_LEFT"):SetInnerWidget(
				ui:Expand("HORIZONTAL"):SetInnerWidget(
					ui:Gradient({r=0.1,g=0.1,b=0.1,a=0.8}, {r=0.0,g=0.0,b=0.1,a=0.0}, "HORIZONTAL"):SetInnerWidget(
						ui:Margin(20):SetInnerWidget(ui:VBox():PackEnd({
							ui:Label(character.name):SetFont("HEADING_NORMAL"),
							ui:Label(character.title):SetFont("HEADING_SMALL"),
						}))
					)
				)
			)
		)
	end,

}
