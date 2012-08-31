local ui = Engine.ui

ui.templates.FileDialog = function (args)
	local title = args.title or "Choose file..."
	local root = args.root or "USER"
	local path = args.path or ""

	print(string.format("FileDialog: title %s root %s path %s", title, root, path))

	local files, dirs = FileSystem.ReadDirectory(root, path)

	local list = ui:VBox()

	for i = 1,#dirs do list:PackEnd(ui:Label(dirs[i].."/")) end
	for i = 1,#files do list:PackEnd(ui:Label(files[i])) end

	local dialog =
		ui:Grid(3, 3)
			:SetCell(1,1,
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({
						ui:Label(title),
						list
					})
				)
			)

    return dialog
end
