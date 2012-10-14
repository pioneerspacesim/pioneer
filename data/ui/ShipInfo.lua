local ui = Engine.ui

ui.templates.ShipInfo = function (args)
	local shipId = Game.player.shipType
	local shipType = ShipType.GetShipType(shipId)

	return ui:Background():SetInnerWidget(ui:Margin(30):SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Label("Ship information"):SetFont("HEADING_LARGE"),
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label("Hyperdrive:"),
								ui:Label("Hyperspace range:"),
								ui:Margin(10),
								ui:Label("Capacity:"),
								ui:Label("Free:"),
								ui:Label("Used:"),
								ui:Label("All-up weight:"),
								ui:Margin(10),
								ui:Label("Front weapon:"),
								ui:Label("Rear weapon:"),
							})
						})
				})
			})
			:SetColumn(1, {
				ui:VBox(10)
					:PackEnd(ui:Label(shipType.name):SetFont("HEADING_LARGE"))
					:PackEnd(UI.Game.ShipSpinner.New(ui, Game.player.shipType))
			})
	))
end
