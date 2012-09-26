local ui = Engine.ui

ui.templates.FileDialog = function (args)
	local title = args.title or "Choose file..."
	local root = args.root or "USER"
	local path = args.path or ""

	local files, _ = FileSystem.ReadDirectory(root, path)

	local list = ui:List()
	for i = 1,#files do list:AddOption(files[i]) end

	local loadButton = ui:Button():SetInnerWidget(ui:Label("Load game"))
	local cancelButton = ui:Button():SetInnerWidget(ui:Label("Cancel"))

	local dialog =
		ui:Grid(3, 3)
			:SetCell(1,1,
				ui:VBox(10)
					:PackEnd(ui:Background():SetInnerWidget(ui:Label(title)))
					:PackEnd(ui:Scroller():SetInnerWidget(list), { "EXPAND", "FILL" })
					:PackEnd(ui:HBox():PackEnd({
						ui:Align("LEFT"):SetInnerWidget(loadButton),
						ui:Align("RIGHT"):SetInnerWidget(cancelButton),
					}, { "EXPAND", "FILL" } ))
			)

    return dialog
end
