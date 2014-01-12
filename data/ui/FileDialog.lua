-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local FileSystem = import("FileSystem")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

ui.templates.FileDialog = function (args)
	local title       = args.title       or l.SELECT_FILE
	local root        = args.root        or "USER"
	local path        = args.path        or ""
	local selectLabel = args.selectLabel or l.SELECT
	local cancelLabel = args.cancelLabel or l.CANCEL
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
	cancelButton.onClick:Connect(onCancel)

	if #files > 0 then
		selectButton:SetEnabled(true)
		list:SetSelectedIndex(1)
	else
		selectButton:SetEnabled(false)
	end

	local fileEntry
	if args.allowNewFile then
		fileEntry = ui:TextEntry()
		if #files > 0 then
			fileEntry:SetText(files[1])
		end
		fileEntry.onChange:Connect(function (fileName)
			fileName = util.trim(fileName)
			selectButton:SetEnabled(fileName ~= '')
		end)
		list.onOptionSelected:Connect(function (index, fileName)
			if fileName ~= '' then
				fileEntry:SetText(fileName)
				selectButton:Enable()
			end
		end)
		selectButton.onClick:Connect(function ()
			fileName = util.trim(fileEntry.text)
			if fileName ~= '' then
				onSelect(fileName)
			end
		end)
	else
		selectButton.onClick:Connect(function ()
			fileName = list.selectedOption
			if fileName ~= '' then
				onSelect(fileName)
			end
		end)
	end

	local content = ui:VBox()
	content:PackEnd(ui:Align("MIDDLE", ui:Label(title):SetFont("HEADING_NORMAL")))
	if args.helpText then
		content:PackEnd(ui:Label(args.helpText):SetFont("NORMAL"))
	end
	if fileEntry then content:PackEnd(fileEntry); end
	content
		:PackEnd(ui:Expand("BOTH", ui:Margin(10, "VERTICAL", ui:Scroller(list))))
		:PackEnd(ui:Grid(2,1):SetRow(0, {
			ui:Align("LEFT", selectButton),
			ui:Align("RIGHT", cancelButton),
		}))

	local dialog =
		ui:ColorBackground(0,0,0,0.5,
			ui:Grid({1,3,1}, {1,3,1})
				:SetCell(1,1, ui:Background(content))
		)

	return dialog
end
