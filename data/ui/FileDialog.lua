-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui
local t = Translate:GetTranslator()

ui.templates.FileDialog = function (args)
	local title       = args.title       or t("Select file...")
	local root        = args.root        or "USER"
	local path        = args.path        or ""
	local selectLabel = args.selectLabel or t("Select")
	local cancelLabel = args.cancelLabel or t("Cancel")
	local onSelect    = args.onSelect    or function (name) end
	local onCancel    = args.onCancel    or function () end

	local ok, files, _ = pcall(FileSystem.ReadDirectory, root, path)
	if not ok then
		print('Error: ' .. files)
		files = {}
	end

	local list = ui:List()
	for i = 1,#files do list:AddOption(files[i]) end

	local selectButton = ui:Button(ui:Label(selectLabel):SetFont("HEADING_NORMAL"))
	local cancelButton = ui:Button(ui:Label(cancelLabel):SetFont("HEADING_NORMAL"))
	selectButton.onClick:Connect(function () onSelect(list.selectedOption) end)
	cancelButton.onClick:Connect(onCancel)

	local dialog =
		ui:Grid({1,3,1}, {1,3,1})
			:SetCell(1,1,
				ui:VBox(10)
					:PackEnd(ui:Background(ui:Label(title):SetFont("HEADING_NORMAL")))
					:PackEnd(ui:Expand("BOTH", ui:Scroller():SetInnerWidget(list)))
					:PackEnd(ui:Grid(2,1):SetRow(0, {
						ui:Align("LEFT", selectButton),
						ui:Align("RIGHT", cancelButton),
					}))
			)

	return dialog
end
