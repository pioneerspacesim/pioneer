local ui = Engine.ui

ui.templates.ShipInfo = function (args)
    return ui:Background():SetInnerWidget(ui:Margin(30):SetInnerWidget(
        UI.Game.ShipSpinner.New(ui, Game.player.shipType)
	))
end
