-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local FileSystem = import("FileSystem")
local Format = import("Format")

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

	-- sort by modification time (most recent first)
	table.sort(files, function (a, b)
		return (a.mtime.timestamp > b.mtime.timestamp)
	end)

	local list = ui:List()
	for i = 1,#files do
		list:AddOption(Format.Date(files[i].mtime.timestamp) .. ' - ' .. files[i].name)
	end

	local selectButton = ui:Button(ui:Label(selectLabel):SetFont("HEADING_NORMAL"))
	local cancelButton = ui:Button(ui:Label(cancelLabel):SetFont("HEADING_NORMAL"))
	cancelButton.onClick:Connect(onCancel)

	if #files > 0 then
		selectButton:Enable()
		list:SetSelectedIndex(1)
	else
		selectButton:Disable()
	end

	local fileEntry
	if args.allowNewFile then
		fileEntry = ui:TextEntry()
		if #files > 0 then
			fileEntry:SetText(files[1].name)
		end
		fileEntry.onChange:Connect(function (fileName)
			fileName = util.trim(fileName)
			if fileName ~= '' then
				selectButton:Enable()
			else
				selectButton:Disable()
			end
		end)
		list.onOptionSelected:Connect(function (index, fileName)
			if fileName ~= '' then
				-- +1 because index is 0-based
				fileEntry:SetText(files[index+1].name)
				selectButton:Enable()
			end
		end)
		selectButton.onClick:Connect(function ()
			local fileName = util.trim(fileEntry.text)
			if fileName ~= '' then
				onSelect(fileName)
			end
		end)
	else
		selectButton.onClick:Connect(function ()
			local fileName = files[list.selectedIndex].name
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
			ui:Grid({1,5,1}, {1,5,1})
				:SetCell(1,1, ui:Background(content))
		)

	return dialog
end
