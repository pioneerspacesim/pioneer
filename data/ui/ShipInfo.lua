local ui = Engine.ui

ui.templates.ShipInfo = function (args)
	return ui:Background():SetInnerWidget(ui:Margin(30):SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(1, {
				ui:VBox():PackEnd(ui:Align("TOP"):SetInnerWidget(UI.Game.ShipSpinner.New(ui, Game.player.shipType)))
			})
	))
end
