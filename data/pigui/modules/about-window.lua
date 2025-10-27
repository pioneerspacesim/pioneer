-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Event = require 'Event'
local Input = require 'Input'
local Game = require 'Game'
local Lang = require 'Lang'
local PiImage = require 'pigui.libs.image'
local Vector2 = _G.Vector2

local l = Lang.GetResource("about")
local lui = Lang.GetResource("ui-core")

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local mainButtonSize = ui.theme.styles.MainButtonSize
local optionButtonSize = ui.rescaleUI(Vector2(100, 32))

local optionsWinSize = Vector2(ui.screenWidth * 0.4, ui.screenHeight * 0.6)

local showTab = 'info'


local function optionTextButton(label, tooltip, enabled, callback)
	local variant = not enabled and ui.theme.buttonColors.disabled

	local button
	ui.withFont(pionillium.medium, function()
		button = ui.button(label, optionButtonSize, variant, tooltip)
	end)
	if button then
		if enabled then
			callback(button)
		end
	end
end

local function mainButton(icon, tooltip, selected, callback)
	local button = ui.mainMenuButton(icon, tooltip, selected)
	if button then
		callback()
	end
	return button
end


local function showInfo()
	ui.withFont(pionillium.heading, function()
		ui.textWrapped(l.ABOUT)
	end)

	ui.withFont(pionillium.body, function()
		ui.textWrapped(l.INFO_BLURB)
		ui.textWrapped(l.INFO_BODY)

		ui.separator()
		ui.spacing()

		ui.bulletText(l.LINK_HOMEPAGE)
		ui.sameLine()
		ui.textLinkOpenURL("http://pioneerspacesim.net/")

		ui.bulletText(l.LINK_BUG_TRACKER)
		ui.sameLine()
		ui.textLinkOpenURL("https://github.com/pioneerspacesim/pioneer/issues/")

		ui.bulletText(l.LINK_COMMUNITY_FORUM)
		ui.sameLine()
		ui.textLinkOpenURL("http://spacesimcentral.com/community/pioneer/")

		ui.bulletText(l.LINK_DEVELOPER_FORUM)
		ui.sameLine()
		ui.textLinkOpenURL("https://forum.pioneerspacesim.net/")

		ui.bulletText(l.LINK_COMMUNITY_CHAT)
		ui.sameLine()
		ui.textLinkOpenURL("Beyond the Frontier", "https://discord.gg/Xzwh5ZPFcK")

		ui.bulletText(l.LINK_DEVELOPER_CHAT)
		ui.sameLine()
		ui.textLinkOpenURL("#pioneer @ libera.chat", "https://web.libera.chat/#pioneer")

		ui.separator()
		ui.spacing()

		ui.textWrapped(l.GAME_RELEASE)
		ui.bulletText(l.VERSION_CURRENT)
		ui.sameLine()
		ui.inputText("##current_version", Engine.version , { "ReadOnly" })

		ui.bulletText(l.VERSION_LATEST)
		ui.sameLine()
		ui.textLinkOpenURL(l.VERSION_LATEST_CONTINUED, "https://github.com/pioneerspacesim/pioneer")

	end)
end

local function showHelp()
	ui.withFont(pionillium.heading, function()
		ui.text(l.DOCUMENTATION)
	end)

	ui.withFont(pionillium.body, function()
		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.LINK_MANUAL, "https://wiki.pioneerspacesim.net/wiki/Manual")

		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.LINK_BASIC_FLIGHT, "https://wiki.pioneerspacesim.net/wiki/Basic_flight")

		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.LINK_FAQ, "https://wiki.pioneerspacesim.net/wiki/FAQ")

	end)
end

local function showDonateInfo()
	ui.withFont(pionillium.heading, function()
		ui.text(l.SUPPORT_DEVELOPMENT)
	end)

	local qr = PiImage.New("icons/donate-qr.png")

	ui.withFont(pionillium.body, function()
		ui.textWrapped(l.DONATE_BODY)
		ui.spacing()
		ui.textLinkOpenURL("PayPal:", "https://www.paypal.com/donate?hosted_button_id=PYPCS2TYD8H4G")
		ui.spacing()
		qr:Draw(Vector2(128, 128))

		ui.spacing()
		ui.text(l.DONATE_OPTIONS)
		ui.sameLine()
		ui.textLinkOpenURL(l.DONATE_OPTIONS_CONTINUED, "https://pioneerspacesim.net/page/donate")

		ui.spacing()
		ui.text(l.DONATE_THANKS)
	end)
end


local function showContribute()
	ui.withFont(pionillium.heading, function()
		ui.text(l.CONTENT_CONTRIBUTION)
	end)

	ui.withFont(pionillium.body, function()
		ui.textWrapped(l.CONTRIBUTE_BODY)

		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.CONTRIBUTE, "https://wiki.pioneerspacesim.net/wiki/How_you_can_contribute")

		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.TRANSLATIONS, "https://wiki.pioneerspacesim.net/wiki/Translations")

		ui.bulletText("")
		ui.sameLine()
		ui.textLinkOpenURL(l.DEVELOPER_DOCUMENTATION, "https://dev.pioneerspacesim.net/")
	end)
end


local function showGPL()
	ui.withFont(pionillium.heading, function()
		ui.text(l.LICENSE)
	end)
	ui.withFont(pionillium.body, function()
		ui.text("Licensed under the terms of the GPL v3")
		ui.text("Copyright © 2008-2025 Pioneer Developers")
		ui.text("Portions copyright © 2013-2014 Meteoric Games Ltd")

		ui.textWrapped("Pioneer's core code and extension modules are licensed under the terms of the GNU General Public License version 3. See licenses/GPL-3.txt for details.")
		ui.textWrapped("Pioneer's art, music and other assets are licensed under the terms of the Creative Commons Attribution-ShareAlike 3.0 Unported License. See licenses/CC-BY-SA-3.0.txt for details.")
	end)
end

local function showSoftwareUsed()
	ui.withFont(pionillium.heading, function()
		ui.text(l.SOFTWARE)
	end)

	ui.textWrapped(l.SOFTWARE_BODY)
	ui.separator()

	if ui.collapsingHeader(l.SOFTWARE) then

		if ui.treeNode("lua") then
			ui.textWrapped("Lua 5.2.1 by R. Ierusalimschy, L. H. de Figueiredo & W. Celes")
			ui.text("Copyright (C) 1994-2012 Lua.org, PUC-Rio")
			ui.text("Licensed under the MIT licence (see contrib/lua/src/lua.h)")
			ui.textLinkOpenURL("https://github.com/ocornut/imgui")
			ui.treePop()
		end

		if ui.treeNode("GLEW") then
			ui.textWrapped("GLEW: The OpenGL Extension Wrangler")
			ui.textWrapped("Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>")
			ui.textWrapped("Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>")
			ui.textWrapped("Copyright (C) 2002, Lev Povalahev")
			ui.textWrapped("Licensed under the Modified BSD licence (see licenses/GLEW.txt)")
			ui.textLinkOpenURL("https://glew.sourceforge.net/")
			ui.treePop()
		end

		if ui.treeNode("atomic_queue") then
			ui.text("atomic_queue v1.6.4 - C++ lockless queue")
			ui.text("Copyright (c) 2019 Maxim Egorushkin")
			ui.text("Licensed under the MIT license (see contrib/atomic_queue/LICENSE)")
			ui.treePop()
		end

		if ui.treeNode("argh") then
			ui.text("Copyright (C) 2016, Adi Shavit")
			ui.text("Licensed under the BSD 3-clause license (see licenses/BSD-3-Clause.txt)")
			ui.treePop()
		end

		if ui.treeNode("miniz") then
			ui.textWrapped("By Rich Geldreich, April 2012")
			ui.text("Public domain (see contrib/miniz/miniz.h)")
			ui.treePop()
		end

		if ui.treeNode("LZ4 Library") then
			ui.text("Copyright (c) 2011-2016, Yann Collet")
			ui.textWrapped("Licensed under the BSD 2-Clause license (see licenses/LZ4.txt)")
			ui.treePop()
		end

		if ui.treeNode("lookup3.c") then
			ui.text("By Bob Jenkins, May 2006")
			ui.textWrapped("Public domain (see contrib/jenkins/lookup3.c)")
			ui.treePop()
		end

		if ui.treeNode("vcacheopt.h") then
			ui.text("Copyright (C) 2009, Michael Georgoulpoulos")
			ui.textWrapped("Licensed under the MIT licence (see contrib/vcacheopt/vcacheopt.h)")
			ui.treePop()
		end

		if ui.treeNode("PicoDDS") then
			ui.text("Copyright (C) 2011 Andrew Copland")
			ui.textWrapped("Includes portions of the Game Texture Library (GTL)")
			ui.text("GTL is Copyright (C) 2005, 2006, 2007 Rob Jones")
			ui.text("Copyright (C) 2006, 2007 Michael P. Jung")
			ui.text("GTL and PicoDDS are licensed under the zlib License")
			ui.textLinkOpenURL("https://github.com/fluffyfreak/PicoDDS")
			ui.treePop()
		end

		if ui.treeNode("JSON for Modern C++") then
			ui.text("Copyright © 2013-2018 Niels Lohmann.")
			ui.textWrapped("Licensed under the MIT License (see contrib/json/json.hpp)")
			ui.textLinkOpenURL("https://github.com/nlohmann/json")
			ui.treePop()
		end

		if ui.treeNode("Base64 Encode / Decode") then
			ui.text("Copyright © 2013 Tomas Kislan, Adam Rudd")
			ui.textWrapped("Licensed under the MIT License (see contrib/base64/base64.hpp)")
			ui.treePop()
		end

		if ui.treeNode("fmt") then
			ui.textWrapped("fmt - open-source formatting library for C++")
			ui.textWrapped("Copyright (c) 2012 - present, Victor Zverovich")
			ui.text("Licensed under the MIT license")
			ui.treePop()
		end

		if ui.treeNode("doctest") then
			ui.textWrapped("doctest - The fastest feature-rich C++11/14/17/20 single-header testing framework")
			ui.text("Copyright (c) 2016-2021 Viktor Kirilov")
			ui.text("Licensed under the MIT license")
			ui.treePop()
		end

		if ui.treeNode("nanosockets") then
			ui.textWrapped("nanosockets - Lightweight UDP sockets abstraction for rapid implementation of message-oriented protocols")
			ui.text("Copyright (c) 2019 Stanislav Denisov")
			ui.text("Licensed under the MIT license")
			ui.treePop()
		end

		if ui.treeNode("Portable File Dialogs") then
			ui.text("Copyright © 2018–2022 Sam Hocevar")
			ui.textWrapped("Licensed under the WTFPLv2 license (http://www.wtfpl.net/)")
			ui.treePop()
		end

		if ui.treeNode("High Performance C++ Profiler") then
			ui.text("Licensed under the MIT license")
			ui.textLinkOpenURL("https://code.google.com/p/high-performance-cplusplus-profiler/")
			ui.treePop()
		end

		if ui.treeNode("nanosvg") then
			ui.textWrapped("Copyright (c) 2013-14 Mikko Mononen memon@inside.org")
			ui.text("Licensed under the zlib License")
			ui.textLinkOpenURL("https://github.com/memononen/nanosvg")
			ui.treePop()
		end

		if ui.treeNode("pcg-cpp") then
			ui.textWrapped("Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>")
			ui.text("Licensed under the Apache License, Version 2.0")
			ui.treePop()
		end
	end

	if ui.collapsingHeader(l.CONTENT) then

		if ui.treeNode("Pionillium Text font") then
			ui.text("Copyright (c) 2015, Pioneer Developers")
			ui.text("Based on Titillium")
			ui.textWrapped("Copyright (C) 2007-2009 Academia di Belle Arte di Urbino - Campivisivi")
			ui.textWrapped("Licensed under the SIL Open Font Licence v1.1 (see licenses/SIL-1.1.txt)")
			ui.treePop()
		end

		if ui.treeNode("Inpionata font") then
			ui.text("Copyright (c) 2015, Pioneer Developers")
			ui.text("Based on Inconsolata")
			ui.text("Copyright (C) 2006, Raph Levien")
			ui.textWrapped("Licensed under the SIL Open Font Licence v1.1 (see licenses/SIL-1.1.txt)")
			ui.treePop()
		end

		if ui.treeNode("Orbiteer font") then
			ui.text("Copyright (c) 2012, Pioneer Developers")
			ui.text("Based on Orbitron")
			ui.text("Copyright (c) 2009, Matt McInerney")
			ui.textWrapped("Licensed under the SIL Open Font Licence v1.1 (see licenses/SIL-1.1.txt)")
			ui.treePop()
		end

		if ui.treeNode("Deja Vu font") then
			ui.text("Copyright (c) 2003, Bitstream Inc.")
			ui.text("parts Copyright (c) 2006, Tavmjong Bah")
			ui.text("License: see licenses/DejaVu-license.txt")
			ui.treePop()
		end

		if ui.treeNode("WenWuanYi Micro Hei Font") then
			ui.textWrapped("Copyright (c) 2008-2009 The WenQuanYi Project Board of Trusteess")
			ui.textWrapped("Copyright (c) 2008-2009, Qianqian Fangi [FangQ]")
			ui.textWrapped("Copyright (c) 2008-2009, mozbug")
			ui.textWrapped("Licensed under the Apache License 2.0 (see licenses/Apache-2.0.txt)")
			ui.treePop()
		end

		if ui.treeNode("WPZOOM Developer Icon Set") then
			ui.text("Copyright (c) 2010 David Ferreira")
			ui.textWrapped("Licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported")
			ui.text("License (see licenses/CC-BY-SA-3.0.txt)")
			ui.textLinkOpenURL("http://www.wpzoom.com/wpzoom/new-freebie-wpzoom-developer-icon-set-154-free-icons/")
			ui.treePop()
		end

		if ui.treeNode("Galaxy colour image") then
			ui.text("Modified version (rotated/converted to PNG)")
			ui.textLinkOpenURL("http://www.spitzer.caltech.edu/images/2353-sig05-010a-Milky-Way-Bar")
			ui.text("Credit: NASA/JPL-Caltech/R. Hurt (SSC)")
			ui.textWrapped("See licenses/Image Use Policy - NASA Spitzer Space Telescope.html")
			ui.treePop()
		end

		if ui.treeNode("Milky Way panorama") then
			ui.textWrapped("Milky Way panorama ('The star map in galactic coordinates')")
			ui.textWrapped("Modified version (dimmed stars, converted to cube map dds)")
			ui.textLinkOpenURL("https://svs.gsfc.nasa.gov/4851")
			ui.text("Credit:")
			ui.bulletText("Ernie Wright (USRA): Lead Animator")
			ui.bulletText("Laurence Schuler (ADNET): Technical Support")
			ui.bulletText("Ian Jones (ADNET): Technical Support")

			ui.textLinkOpenURL("Public domain", "https://svs.gsfc.nasa.gov/help/")
			ui.sameLine()
			ui.textWrapped("under \"Copyright and Credit Information\"")
			ui.treePop()
		end
	end

	if ui.collapsingHeader(l.ACKNOWLEDGEMENTS) then
		ui.textWrapped("The Pioneer Developers would like to acknowledge the following fine products and services, without which Pioneer would not be possible")

		ui.bulletText("SDL")
		ui.sameLine()
		ui.textLinkOpenURL("http://www.libsdl.org/")

		ui.bulletText("FreeType")
		ui.sameLine()
		ui.textLinkOpenURL("http://freetype.org/")

		ui.bulletText("Ogg Vorbis")
		ui.sameLine()
		ui.textLinkOpenURL("http://vorbis.com/")

		ui.bulletText("libsigc++")
		ui.sameLine()
		ui.textLinkOpenURL("http://libsigc.sourceforge.net/")

		ui.bulletText("assimp")
		ui.sameLine()
		ui.textLinkOpenURL("http://assimp.sourceforge.net/")

		ui.bulletText("MXE")
		ui.sameLine()
		ui.textLinkOpenURL("http://mxe.cc/")

		ui.separator()

		ui.bulletText("GitHub")
		ui.sameLine()
		ui.textLinkOpenURL("https://github.com/")

		ui.bulletText("SourceForge")
		ui.sameLine()
		ui.textLinkOpenURL("https://sourceforge.net/")

		ui.bulletText("randomColorSharped")
		ui.sameLine()
		ui.textLinkOpenURL("https://github.com/nathanpjones/randomColorSharped")

		ui.separator()

		ui.bulletText("OpenGameArt")
		ui.sameLine()
		ui.textLinkOpenURL("http://opengameart.org")
	end
end


local optionsTabs = {
	info=showInfo,
	help=showHelp,
	donate=showDonateInfo,
	contribute=showContribute,
	license=showGPL,
	software=showSoftwareUsed,
}

ui.aboutWindow = ModalWindow.New("Options", function()
	ui.horizontalGroup(function()

		mainButton(icons.info, l.ABOUT, showTab=='info', function()
			showTab = 'info'
		end)

		mainButton(icons.about_questionmark, l.DOCUMENTATION, showTab=='help', function()
			showTab = 'help'
		end)

		mainButton(icons.money, l.SUPPORT_DEVELOPMENT, showTab=='donate', function()
			showTab = 'donate'
		end)

		mainButton(icons.crowd, l.CONTENT_CONTRIBUTION, showTab=='contribute', function()
			showTab = 'contribute'
		end)

		mainButton(icons.legal, l.LICENSE, showTab=='license', function()
			showTab = 'license'
		end)

		mainButton(icons.planet_grid, l.SOFTWARE, showTab=='software', function()
			showTab = 'software'
		end)

	end)

	ui.separator()

	-- I count the separator as two item spacings
	local other_height = mainButtonSize.y + optionButtonSize.y  + ui.getItemSpacing().y * 4 + ui.getWindowPadding().y * 2
	ui.child("options_tab", Vector2(-1, optionsWinSize.y - other_height), function()
		optionsTabs[showTab]()
	end)

	ui.separator()
	optionTextButton(lui.CLOSE, nil, true, function()
		ui.aboutWindow:close()
	end)

end, function (_, drawPopupFn)
	ui.setNextWindowSize(optionsWinSize, 'Always')
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

function ui.aboutWindow:changeState()
	if not self.isOpen then
		self:open()
	else
		self:close()
	end
end

function ui.aboutWindow:open()
	ModalWindow.open(self)
	if Game.player then
		Input.EnableBindings(false)
		Event.Queue("onPauseMenuOpen")
	end
end


return {}
