These release announcement notes includes notable changes relevant to the end player sourced and elaborated on from Changelog.txt. It does not include internal changes to code, refactoring, build system changes listed in said file.



# Pioneer 2023-03-14

This is a bug fix release to compliment the February release. Most noticeable bug, now fixed, was that weapons firing were out of sync making combat exceedingly difficult.


### System editor launching
The February release also included a system editor, but many did not realize it had to be launched [from the command line](https://wiki.pioneerspacesim.net/wiki/Custom_Systems) with proper arguments to specify if you want the model editor or the system editor. There is now a graphical option to select which. The editor window is now resizable as well.


### Gas giant atmosphere / scooping
Some bugs are like roaches - and refuse to go away. We thought the missing atmospheres were fixed but turns out the February release only fixed the procedurally generated planets, not the scripted ones (i.e. the 20 core systems), causing fuel scooping on gas giants in these systems (like Jupiter and Saturn) to not work.


### Zombie traders
If you stayed in a system for a long time you might have noticed performance issues. This was due to several issues with trade ships causing them to accumulate, which have now been fixed.


## Major changes and new features
* Editor Welcome Screen: don't require command line argument (#5772)
* Resizable Editor Window (#5779)

## Bugfixes / Tweaks
* Fix projectile and beam weapons not able to hit, frame sync (#5767)
* Fix the accumulation of zombie traders and some other unnecessary things (#5783)
* Local missions should list distances in AU rather than Ly (#5756)
* System Editor Fixes (#5787)
* Generate atmospheres for gas giants also in legacy custom systems (#5766)
* Minor model fixes (Lodos, Coronatrix) (#5782)
* Adding landing spike to Mola Ramsayi collision mesh (#5785)
* Fix star background generation (#5758)
* Fix path of pioneer executable (#5743)
* Fix 'nil' in hyperdrive Maintenance BBS advert (#5754)
* Fix "bad argument" error in SearchRescue.lua in custom systems (#5769)
* Fix crash when deleting a flightlog entry (#5768)
* Don't crash when there are few background stars (#5778)
* Fix spelling / grammar in English language files (#5780)
* Fix that sometimes ships get stuck the moment they start to dock (#5127)
* Consider InputBindings::Axis active only if it's value is non-zero (#5795)
* Fix incorrect position of starports (#5793)
* Fix system editor loading body parameter wrongly from .json file (#5792)
* Fix incorrect city locations (#5744)
* Fix hyperjump streaks falling down (#5770, #5794)
* Fix invalid binary orbits (#5764, #5794)
* Fix negative commodity demand (#5784, #5794)
* Improve collision mesh to Natrix (#5796)

## Internal Changes
* Add documentation to make life for package maintainers easier (#5749)
* Adding editor.txt, removing ModelViewer.txt to document new editor (#5737)
* Fix MSVC CI build on non-PR refs (#5733)
* Update libfmt, fix compilation warnings and invalid JSON CustomSystem position (#5759)
* Fixed broken screenshots links (#5761)
* Misc cleanup, fixes, and Lua function library improvements (#5773)
* Update Github Upload-Artifact action for Windows CI (#5781)
* Add a recovery reader for the v90 flight log (#5763)
* Add Lua "Persistent Object" Serialization (#5774)



# Pioneer 2024-02-03

https://github.com/pioneerspacesim/pioneer/assets/4182678/25acaa21-61d7-4032-8a2d-023096ea3af1

(Watch on [Youtube](https://www.youtube.com/watch?v=J8gghBtXgsQ) | Music: Distant Home by Diduuz)

A year in development brings with it plenty of new exciting features, but also many changes under the hood which are necessary to lay the groundwork for what is to come.

### New Game Window
New games are now launched from a new menu that exposes the starting state. Players can now fully see what conditions they will be starting in, which also allows developers to initialize a game with parameters of interest to test. The New Game Window also laid the technical groundwork for recovering old saves from the 2023-02-03 release, more on this further down.

![New game window](https://github.com/pioneerspacesim/pioneer/assets/4218491/5bf4a015-6537-487c-9876-f8b603939579)

### Savefile Recovery
Due to Pioneer's partially procedurally generated content, a new release would usually break old game saves. This occurs because sampling the random number generator differently causes the resulting universe to be different, which could cause the space station the player was docked at to no longer exist. This forced each such release to block existing saves from loading. Well no more! As of this release there is now a mechanism that lets players recover old save games, by starting a new game with the progress of the saved one, where variables that could not be resolved become free for the player to set manually. Note: active missions are not recovered, finish them in your old Pioneer install if you want to, before loading the save into this new Pioneer version.

### Manual landing on orbital stations
From now on you can land on the orbital stations manually, and the autopilot handles landings much more gracefully too. Although it can be done without any flight assist, it is recommended to use the cruise control and rotation matching functions of the flight computer to avoid fender-benders.

### System Editor
This release packs a GUI System Editor, that lets anyone customize existing procedurally-generated systems or design entirely new systems without editing any code. The development team would love if the players create new custom systems and contribute them for inclusion into the core game. This also means all custom systems should now be authored in JSON, through the GUI tool, rather than via hand-written lua-files, which are now deprecated. See [wiki](https://wiki.pioneerspacesim.net/wiki/Custom_Systems) for documentation.

![System Editor](https://github.com/pioneerspacesim/pioneer/assets/4218491/1957a7d0-d2fa-4d79-a0da-e11ea3374be0)

### FlightLog re-worked
For those players who like to take notes about their career as a galactic merchant or mercenary, the flight log has been overhauled. It now supports sorting, filtering, and exporting the flight log to an HTML file.

### Atmosphere Scattering (Experimental)
New in this release is an alternate rendering method for planet and gas giant atmospheres which calculates the real-life Rayleigh / Mie scattering equations to produce more realistic atmosphere colors under all conditions. The version in the release is still experimental - the differences between e.g. a Nitrogen-Oxygen and a Methane atmosphere are not taken into account, and distant terrain rendering is still using the old atmosphere calculations. To turn it on, scroll through the _scattering_ menu under _settings_.

![Atmospheric scattering sunrise](https://github.com/pioneerspacesim/pioneer/assets/4218491/10185229-5c70-4b3b-b0f9-70fdaeaeb7b0)

### And More!
There are many other changes, as is evident from the list below. Some notable are new textures for one of the the ground stations, planet rings being in shadows of the planet, trade ships now being less reckless when hyper jumping out, mission deadlines re-balanced, three new music tracks, and planned hyper jump route is now saved across sessions.

There are also many changes under the hood, but for brevity we refrain from listing them. Briefly, they include things like: code optimizations, render optimizations, improved compilation time and new [API functions](https://codedoc.pioneerspacesim.net/) for modders to use. This will also be the first releases that provides an AppImage. See [Changelog.txt](https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt) for a complete list of all changes.

### Project Infrastructure Changes
On the development team side, shortly after the last release (2023) robn handed over the all the infrastructure that he had been running for us for free. The change meant we now have new URLs to [dev forum](https://forum.pioneerspacesim.net/), [wiki](https://wiki.pioneerspacesim.net/), [API docs](https://codedoc.pioneerspacesim.net/) and a new [dev docs](https://dev.pioneerspacesim.net) page. Our forum and wiki are now hosted on a paid service, with annual cost of $75. If you want to contribute to the war chest, no [donation](https://pioneerspacesim.net/page/donate/) is too small.

### Build Tooling
With this release, we've re-organized our CMake build scripts slightly, added CMake Presets, and introduced a new `PROJECT_VERSION_INFO` variable to better identify the contents and version number of the build when triaging issues. For Windows and Linux developers using Visual Studio, VSCode with the CMake extension, or any IDE that supports CMake Presets, compiling the game should now be only a few clicks away.

Distribution packagers that build Pioneer without Git repository information available (or apply patches in a way that changes the git commit hash) are encouraged to set the PROJECT_VERSION_INFO CMake variable to a value that clearly identifies the source version the package is built from.

Fly safe!

## Major changes and new features
* Implement graphical editor for creating custom systems (#5625)
* Add Model Viewer Widget (#5613)
* Add New Game interface to customize starting parameters (#5561)
* Add a function to recover old versions of saves (#5706)
* Search bar for save/load window and crashes fix (#5634)
* Ship market comparison colors (#5632)
* Sensibly generate high-gravity planets (#5592)
* Improved and cleaned up facegen parts (#5537)
* Docking with orbital in manual mode (#5619)
* Newground station cleanup and new textures (#5704)
* Realistic Rayleigh/Mie atmospheric scattering (#5617)
* Improve planet rings shaders (#5708)
* Update/rework of the flightlog (#5666)
* Add new music tracks by diduuz (#5727)

## Minor changes and features
* Allow double click on an item to load the saved game (#5652)
* Added icon to indicate up/down camera view (#5600)
* SecondHand - Add possibility to refuse selling (#5593)
* Include ad title in bulletin board search (#5589)
* Combat mission explanation about special equipment (#5588)
* ScanManager improvements and ship properties fixes (#5635)
* Unify / re-balance mission travel time implementation (#5707)
* Stop TradeShips from performing illegal Hyperjumps (#5647)
* Improve realism of randomly-generated systems (#5622)
* Add delete button in save/load window (#5674)
* System overview icon upscale and update (#5540)
* Make ship market comparison colors colorblind friendly (#5715)
* Save and load the current planned hyperjump in the save file (#5721)

## Bugfixes / Tweaks
Bugfixes are not necessarily relative previous 2023 February release, but rather reflects fixing features as they go into master branch.

* Fix installed directory structure on Windows (#5686)
* Correct value of g-force in system editor (#5658)
* Fix assertion failed in Sfx::Sfx(const Json &jsonObj) (#5668)
* Fix crash in l_body_get_ground_position if there is no body to get (#5665)
* Fix lua components corruption on save (#5657)
* Disable scrolling for tab-view and character info (#5642)
* Fix failed build for ALT Linux, by calming down -Werror=return-type (#5649)
* Fix for glued cockpit (#5631)
* Fix for commodity market in stock and demand reset (#5633)
* Fuel scooping rework (#5609)
* Fix Scans not resuming on game continue (#5577)
* Fix disappearing fuel after save and load (#5534)
* Fix ship info screen showing wrong cargo capacity and usage (#5557)
* Fix for "Assertion failed!" in the "Ship Repairs" window (#5640)
* Wording change for commands in the "Crew Roster" window (#5641)
* Fix constant planet density vs radius ratio (#5592)
* Fix main menu music not playing after exiting to main menu (#5645)
* Fix incorrect time acceleration after closing pause menu (#5646)
* Fix absurd number of spaceports generated on high-population worlds (#5625)
* Fix surface ports being generated on a single "orbital line" around the body (#5625)
* Ignore x-axis only mouse wheel events (#5627)
* Fix for rounding error in lobby's +/-10% buttons (#5628, #5630)
* Fixes after sector map refactoring and new start menu added (#5612)
* Fix issues when quitting Lua console (#5597)
* Mouseover tooltip for the ECM and the Advanced ECM systems (#5590)
* Fix SIGFPE from zero length vector if flying long time enough (#5569)
* Fix military drive being too heavy (#5580)
* Fix empty accessory files used to reduce facegen accessory spawn chance (#5558)
* Fix autopilot crash with no body in frame of reference (#5551)
* Fix crashes when ships jump into system from hyperspace (#5564)
* Fix set hyperspace target button missing from SAR mission (#5531)
* Reduce chance of autopilot crashing into planets between ship and target (#5481)
* Avoid misaligned read when loading SGM files (#5692)
* Remove transparent backgrounds from screenshots (#5660)
* Fix assert on startup when calling back on an empty string (#5717)
* Fix orbital / surface scan missions being unable to be completed (#5724)
* Fix player being unable to collide with Stations if only ship in Frame (#5720)
* Crash from hyperjump-planner indexing local ship (nil) #5725
* Fix missing gas giant atmospheres (#5728)
* Fix undefined behaviour in CommandBufferGL.cpp (#5694)
* Fix mission data for Custom Cargo mission persisting between games (#5730)
* Fix model viewer not working (#5729)
* Fix bogus GPS coordinates when entering an orbit (#5729)
* Fixes Visual Studio CMake build only supports Debug, not release or profiling (#5729)



# Pioneer 2023-02-03

We are happy to bring you a release with many new exciting features and improvements: a new gorgeous ship, the _Coronatrix Courier_, two new cockpits, and a redesigned default cockpit for all other ships. City generation is now more efficient, and city layout produces much larger cities. Some station shipyards now have a paint shop, so you can give your rusty old space-bucket a custom paint job.

![from_cockpit_1](https://user-images.githubusercontent.com/619390/215580236-ee7c4484-9fe9-4872-be7a-b326f7cf6a19.png)
(A Blender render of the new Coronatrix and its cockpit)

![](https://user-images.githubusercontent.com/4182678/169698135-ce90136a-6f89-46c3-b68e-b7bb5ac064c7.png)
(The new Xylophis cockpit, seen from in-game at Barnard's star)

![image](https://user-images.githubusercontent.com/4218491/216178567-4e66aa84-d881-444c-9aa1-100b9a93a747.png)
(Mexico City in the morning)

The economy has seen a re-write to now also allow interplanetary trading: stations consume and produce goods, and demand will affect price, causing profitability of an over-used trade route to drop until the market returns to equilibrium.

After 11 years in hiatus, the Scout mission is back, now with special ship equipment, letting you take missions to scan a planet from meters above the surface or from orbit.

Also, our Info and Station screens now have a more consistent UI font size. Please also make sure to check your keybindings in the settings menu, and make note of new changes.

Long-range combat has been improved, by making missiles much smarter, and allowing beam lasers to aim where they should. Ships are now easier to control due to a new speed limiter and much improved cruise modes, and support for cockpit head-tracking and mouse zooming.

https://user-images.githubusercontent.com/18342621/168445666-df6de4c5-ba58-4a3d-a531-29681b64516b.mp4

(Showing off the new ship control improvements, during development)

On the administrative side of things, robn has handed over his last responsibilities, which caused us to change the [dev forum URL](https://forum.pioneerspacesim.net), and create a new [wiki for developer documentation](https://dev.pioneerspacesim.net/).

**NOTE:** this release version is not backwards-compatible with old savefiles due to major breaking changes to multiple gameplay systems.

Fly safe!

## Major Changes and New features
* New ship: Coronatrix Courier (remodel and conversion of the Amphiesma) (#5462)
* Add custom two-seater cockpit for the Xylophis (#5373)
* New, more detailed default cockpit model (#5368)
* Add bespoke cockpit for the Sinonatrix (#5370)
* Enable in-system trading between different stations (#5474)
* New mission type: perform orbital or surface scans of a body for large rewards (#5433)
* Add station stock and demand, with impact on commodity prices (#5474)
* New sidebars and cargo display in the Flight UI (#5431)
* Massively expanded and improved spaceport city generation (#5430)
* Improved lead calculation for pulse and beam weapons (#5417)
* New follow-target and follow-orient flight control modes (#5371)
* Add player-controllable flight speed limiter (#5371)
* New radial menu for autopilot hold-orientation functions (#5371)
* New paintshop customization interface for stations (#5342)
* Adding the new and fixed patterns and texture to ships (#5347)
* Display thruster plumes in cockpit view for enabled ships (#5370)
* Metal creaking sound feedback under high acceleration (#5335)
* Add cockpit headtracking, mouse zoom and smoothing (#5460)
* Improved missile targeting to make them significantly more dangerous (#5472)


## Minor Changes
* Implement select target and cycle hostile contact buttons (#5429)
* Nav target icons indicate whether the target is on the other side of a body (#5438)
* Display station tech-level in System Map info panel (#5439)
* Added more varied and interesting donation mission titles (#5445)
* Anti-aliased rendering of ship ID labels (#5459)
* Player starts with a Coronatrix at sol (#5462)
* Add interface to configure joystick axes and deadzones (#5477)
* Allow treating joystick axes as "half axes" (e.g. gamepad triggers) (#5477)
* Increase trade-in value of ships from 50% to 65% (#5475)
* Increase amount of credits granted at Mars and New Hope starts (#5474)
* Load mods from unpacked directories as well as zip files (#5432)
* Improve LuaTimer per-frame performance (#5453)
* Allow model files to reference textures in other folders (#5459)
* Add a model hierarchy view to the ModelViewer (#5459)
* Fuel / cargo scooping is now moddable by Lua (#5389)
* Cargo life support expiration is now handled in Lua (#5389)
* Refactor input binding widget to allow binding key chords (#5371)
* Add axis input to select radial menu options with gamepad (#5371)


## Bugfixes / Tweaks
There have been many bugs fixed see, for a full list, see [Changelog.txt](https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt), some of the more notable:

* Increased equipment capacity and tank size of Sinonatrix (#5472)
* Fix hostile ships running away from the player and never engaging (#5436)
* Fix saves with invalid ships causing menu errors (#5480)
* Fix tradeships becoming unresponsive and filling all station pads (#5456)
* Fix shading of Vlastan Library building (#5483)
* Fix lodos missing a gun mount tag (#5486)
* Fix station pads being assigned wrong bay indicies (#5488)
* Fix undefined behavior when clearing joystick axis bindings (#5415)
* Fix joystick axis reporting invalid joystick value when cleared (#5415)
* Fix Search and Rescue (SAR): fix refueling mission (#5383)
* Fix SAR allowing ships without suitable passenger capacity (#5413)
* Fix SAR missions generating invalid configurations (#5446)
* Fix SAR missions not giving the player enough time to reach the target (#5446)
* Fix potential to buy more than the available amount of a commodity (#5413)
* Fix crash when opening System View in hyperspace (#5428)
* Fix MusicPlayer crashing when music ends on sector view while in hyperspace (#5425)
* Fix cargo life support not being correctly handled (#5419)
* Fix catastrophic rotation after undocking from space station (#5422)
* Fix ship having no gun cooling after selling laser cooler (#5426)
* Fix shield recharging being disabled after selling shield booster (#5434)
* Fix broken Scoop mission, due to onCargoDestroyed event not triggered (#5407)
* Fix illegal scoop missions didn't spawn saleable goods (#5446)
* Fix randomly-generated background stars being invisible (#5390)
* Fix docked music not playing (#5378)
* Fix Flight Log use game start time instead of the Jan 1 3200-based Game.time (#5263)
* Fix Flight Log not being saved (#5498)
* Fix ship passenger cabin capacity not being shown when buying a ship (#5439)
* Fix SolFed homeworld was pointing at Shanghai instead of Mars (#5439)
* Fix auto-route button routing to the wrong body in the target system (#5435)
* Fix high-priority messages not interrupting timewarp (#5446)
* Fix incorrect message when landing at ground stations (#5450)
* Fix an issue that duplicated BBS adverts when autosaving (#5456)
* Fix several crashes when changing player / model debug flags (#5459)
* Fix UI error when pumping fuel in hyperspace (#5455)
* Fix UI tooltips having extremely inconsistent font sizes (#5454) (#5475)
* Fix Terrain generation bug, making high altitude terrain lack detail (#5498)
* Fix memory leaks (#5495)
* Fix Skipjack textures (#5496)
* Fix no crew at start (#5515)
* No negative times in BBS (#5515)
* Fix distance to planet surface being calculated relative to body center (#5515)
* Default flight roll keys to Q/E (#5515)
* Armed recon mission informs the player when they've reached the target area (#5522)



# Pioneer 2022-02-03

The Pioneer developers have been busy since the 2021-07 summer release, and have made several major improvements to the game! Perhaps the most notably, the "[oldUI](https://pioneerwiki.com/wiki/GUI_introduction#Background_history)" interface system has been completely removed from the codebase, replaced with a massively improved label rendering system for the Sector Map and the old System Overview screen being completely rewritten into the new System Atlas screen.

![2022-01-30-100714_1600x900_scrot](https://user-images.githubusercontent.com/619390/151693692-4647fb5a-b170-4c51-b960-4b4e575deb7c.png)
![2022-01-30-100914_1600x900_scrot](https://user-images.githubusercontent.com/619390/151693698-84c1b2a8-86d2-4517-a705-dac8ec8cec64.png)

Similarly, the station Bulletin Board and Ship Equipment displays have been redesigned to show more pertinent information to the user and pave the way for future usability features.

This release features a major gameplay improvement: commodity stocking at stations has seen a refactor to remove an artificial profit-cap, which provides players with an increased economic incentive for flying larger bulk-cargo ships and finding a profitable commodity route tailored to their ship. We've also added a first iteration of dynamic stock depletion; in its current state it serves as an incentive for bulk traders to rotate between different stations when they have purchased all available stock of a commodity. This will be expanded upon in further releases!

On the graphical side of things, an internal refactor of Pioneer's rendering subsystem has slightly improved performance in drawcall-heavy scenes and opened the doors for developers to implement more complicated graphical effects. Following on from the last release, another ship has received a complete graphical overhaul, this time the Xylophis light shuttlecraft! It's available as a starting ship in the Barnard's Star scenario, and can be a quite economical package courier in the early stages of the game.

![](https://user-images.githubusercontent.com/4182678/149630066-d3b8d6cc-0635-41a8-8fe6-d07f3b1dec7d.png)

As always, a list of noteworthy changes and bugfixes is collected here; the full list is available in the [Changelog.txt](https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt) file distributed with the game.

## Major Changes
* New galaxy skybox, improved star brightness and density (#5124)
* Redesign the BBS layout, show pertinent mission info (#5312)
* New slot-based ship equipment display (#5315)
* Completely new System Atlas mode for System Map (#5239)
* Add System Overview Widget to System Map view (#5327)
* Xylophis overhaul, new model (#5323)
* Name generation expanded, and make first + last name match (#5223)
* Rebalance goods restocking & goods availability, allowing bulk trading (#5291)
* Draw sector map labels with ImGui. (#5247)
* Rewrite internal renderer API, improve performance with many draw calls (#5156)

## Minor Changes
* Default language is based on the user's environment language code (#5326)
* Unify default UI theming, ensure UI scaling snaps to pixel values (#5315)
* Change CommsWindow opacity, adjust Quit message window (#5303)
* No scrollbar on short comms log, show newest message first (#5256)
* Double-clicking the pause button opens options menu (#5279)


## Bugfixes / tweaks
* FuelClub: Refuel internal fuel tank only once a day (#5311)
* Fix mechanic character not being persistent (#5306)
* Fix clicks falling through buttons in Sector Map (#5324)
* Make fuel scooping slightly easier and safer (#5281)
* Fix sharp edges on hypercloud halo (#5288)
* Fix NPC ship trajectory calculations crashing into planets (#5250)
* Fix DebugRPG menu crashing when current game is ended (#5292)
* Fix gun tag on Skipjack caused issues after save/load (#5269)
* Fix sold out Illegal commodity price not at x2 over black market (#5277)
* Fix crash from selling to sold out advert when amount=0 (#5262)
* Re-added cargo mission tonnage dialogue option (#5228)
* Fix FlightLog buttons being offscreen (#5224)
* Fix ESC key not closing the settings window (#5224)
* Fixed DSMiner ship vertex normal issues (#5229)
* Fix crash in ModelViewer when switching to FPS navigation (#5224)



# Pioneer 2021-07-23

We hope you will enjoy this summer release, bringing several major additions, most notably a total visual overhaul of the Sinonatrix, a total rebalance of cost and maneuverability for all ships, and systems now filled with merchants, busily making their way to their places of commerce, making pioneer a little less "lonely". As usual, we have made many tweaks and other additions, and hopefully more bugs fixed than new ones introduced.

## Major Changes and New Features:

* New ship model: the OPLI Sinonatrix has received an all-new hull courtesy of Nozmajner. This model rework is the first of a series of visual improvements coming to Pioneer, and paves the way for a new ship workflow tailored towards generating much higher-quality content! (#5106)
* Completely rebalanced acceleration, thrust, and price across all ships, and rework many ships' cargo and fuel capacities to have more coherent and physically-informed values. (#4970)
* Improved rendering quality: ship models no longer display spotty white lines at seams, and should now appear crack-free. (#5157)
* Added a custom mission and content related to the Rondel system... you'll have to find it for yourself in-game! (#5095)
* Add music to the map view and add several new music tracks (#5093)
* Add many more tradeships to in-system traffic and improve their behavior (#4984)
* Highly improved performance with many ships in one system (#5166)
* Cargo missions allow negotiating cargo amount with mission giver (#5164)
* Selecting systems by moving around in the Sector Map is now much more reliable (#5227)

## Notable Bug Fixes and Minor Changes:

* Fixed GasGiant and other large planets 'clipping' out of view when seen from far away (#5157)
* Fixed a crash when loading invalid AI command data from savefiles (#5167)
* Fix a bug preventing the player from selecting stars with the same name in sector map search (#5193)
* Fix news event not significantly changing stock and price on the commodity market (#5192)
* Upgrade Pioneer source code to C++17 (#5151)
* Improved warm startup times by multi-threading some asset loading (#4951)

As always, the full list of features, changes, and fixes can be found in Changelog.txt.



# Pioneer 2021-02-03

First official release in a year, on the day, so suit up, and jump into your ship for an epic adventure into the infinite depth of space. As usual, please see [changelog](https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt) for the full list of features, code improvements, and bug fixes, but a selection focusing on what is noticeable for seasoned fellow pioneers follows.


## Major changes and new features
Among the major changes, that you'll hopefully notice in-game, is the move of our UI to the more flexible Imgui library (which we call "PiGui"), something that's been ongoing for years, is now almost done. Another, is vastly improved combat, through aim assist; as well as improved trading, through completely re-balanced commodity prices. Something that might throw off seasoned players: we now have consistent use of MMB (Middle Mouse Button) for rotating camera in both worldview (i.e. cockpit) as well as in sector- and system view, rather than RMB. Other notable milestones are:

- New ship: Skipjack Courier from OKB Kaluri (#4871)
- New model: Escape pods (initially only to be scooped up, not purchasable) (#4877)
- Improve combat targeting by adding aim assist for player weapons (#5037)
- Complete overhaul of commodity prices, to be similar to Frontier (#4831)
- Redesign System and Sector Map View layouts (#4852)
- NewUI is dead, long live PiGui! (#5032)
- More lively station traffic control communications (#4987)
- Ship-specific atmospheric pressure limits (#4958)
- Add surface impact alerts (#4891)
- Captains log added (#4795)
- Show background stars in system view (#5068)
- Show surface starports in system view (#5060)
- Significantly reduce savefile sizes (#5075)
- New BBS advert for soldout commodity (#5059)


## Minor changes
- Refactor the ModelViewer to use PiGui (#4849)
- Convert Police screen to PiGui (#4790)
- Port Crew Roster to PiGui (#5022)
- Port Active Missions display to PiGui (#5025)
- Move ShipRepair screen to PiGui (#4791)
- Move Bulletin Board view to PiGui (#4775)
- Move economy & trade to pigui (#4067)
- Move the System Map to PiGui (#4821)
- Move Economy & Trade -view to pigui (#4967)
- Move planar radar widget to pigui (#5081)
- Move comms to entirely in lua (#5008)
- New unified color theme (42 semantic aliases) and new icons (#4993)
- Add theme color display/editor to debug menu (#4979)
- Make thruster upgrades availabile based on tech level (#4956)
- Stars that are brighter are now bigger and have a brighter colour on the Starfield (#4833)
- Make police non-persistent in Goodstrader, like in Frontier (#4796)
- Recon/combat missions now require radar (#4916)
- Add Star's End system on the other side of the galaxy (#4873)
- Merge 36 Ophiuchi & Gliese 664 into a single star system (#4874)
- Show the ship's translated name in savegame stats (#4953)
- Plentiful tweaks to sector map (#4906)
- Make size of load/save dialogue window sane (#4912)
- Reduce hydrogen price back to 1 credit (#4859)
- Update advice for Goodstrader, reflecting new behaviour (#4824)
- Move ship jump state strings to translation system (#4814)
- Re-introduce support for remote Lua console (#4799)
- Make continue button load _quicksave if autosave not active (#4758)
- Commodities and Economies are now defined in JSON (#4944)
- Add tag display to ModelViewer (#5010)
- Add a selection highlight to the ship market list (#4999)
- Overhaul body names in the 1 Orionis system (#5042)
- Use new icons in Worldview (#5042)
- Show commodity import/export information in market (#5082)
- Update several system's names to use alternate name feature (#5083)
- Adjust mission payouts to more natural numbers (#5050)
- Improved missile damage calculations, slightly boosted damage (#4927)
- Add menu and ingame music from franzopow (#5027)
- Added lower-cost 5MW mining laser (#5037)
- Buffed local delivery mission payouts, reduced fuel costs (#5037)
- Decrease the maximum background star size to 0.3 (#4991)
- Reduce probability for imported goods to be sold out (#5074)
- Donate to cranks advert improvements (#5073)
- Scottish place name revision (#5052)
- Many fixes to system map (#5123)


## Bug fixes
A selection of some of the more notable/important bugfixes, that we hope you never experienced in the first place.

- Fix starting a new game charging a docking fee (#5000)
- Fix radar display state being incorrectly saved (#4999)
- Fix jump range being improperly persisted across games (#4999)
- Fix ship displays not properly updating between games / selections (#4999)
- Fix tradeships inflating save files with 1000s of duplicate cargo items (#4993)
- Fix external camera causing segfaults/crashes under some circumstances (#4999)
- Fix saved games in external camera resetting to internal camera on load (#4999)
- Fix zenith indicator not pointing away from the planet (#4999)
- Allow all ships with hyperdrives to be selected for rendezvous missions (#5002)
- Clear starports array if system has no population (#4950)
- No more crashes when hyperjumping (#4907)
- Fix ship directional indicators not pointing in the correct direction (#4941)
- Fix requring a camera frame when drawing PiGui (#4941)
- Fix being able to load invalid save versions (#4945)
- Fix compilation on i686 builds (#4945)
- Fix atmospheric flight calculations (#4946)
- Fix buying commodities not subtracting station stock (#4909)
- Fix hyperdrive last service date being wrong (#4910)
- Fix player sometimes exiting hyperspace inside a star (#4905)
- Correctly restart mission timer when loading saves (#4870)
- Remove price reduction when selling commodities (#4876)
- Clear SetSpeedTarget when jumping (#4880)
- Commodity name clean-up (#4875)
- Prevent unwanted font face changes (#4882)
- Fix increase / decrease buttons in SystemView being linked (#4868)
- Fix UB related to allocation/free mismatch (#4867)
- Prevent change of Pioneer's mouse pointer when hiding HUD (#4827)
- Fix body grouping and setspeed target behaviour in flightUI (#4794)
- Fix starfield not taking player's location into account (#4838)
- Improve Mouse Capture Handling, fix UI deadlock (#4842)
- Fix unused/free cabins shown in station footer being wrong (#4808)
- Small additional improvements to orbit calculation (#4784)
- Optimize route rendering (#5033)
- Fix crash when switching to system map after hyperjumping (#5077)
- Fix buy/sell of station stock updating inverted (#5072)
- Fix ambient music looping (#5061)
- Fix grammar in translation string for Earth/Sol radius (#5053)
- Be consistent with use of nuclear vs radioactive (#5057)
- Fix menu music issues (#5047)
- Several map fixes and small tweaks (#5048)
- Fall back to an empty resource if it hasn't been translated (#5011)
- Fix Search & Rescue crash where IsPlayer() is falsely called by non-ship body (#4985)
- Minor bugfixes for cargo mission (#4998)
- Same fontsize for ship info view as the other screens (#5024)
- Fix game crash when opening system view in hyperspace (#5120)
- Fix game crash "attempt to index field ?" when docking (#5119)
- Fix section of skybox being void of stars (#5116)
- Fix ambient planet sound playing after leaving surface (#5115)
- Fix exotic systems triggering crash when set at destination (#5114)
- Fix game crash when clearing sold out adverts (#5097)
- Fix sold out advert title wrong after loading saved game (#5092)
- Fix Skipjack collision box for landing compatibility (#5087)
- Fix many small UI issues (#5085)

Big thanks to developers, contributors, and players for keeping this going!



# Pioneer 2020-12-22

What's this, *two* Pioneer releases in one year? We have so many new things coming up that we decided to push out an early testing release ahead of the yearly Pioneer Day release.

Some (but by no means all) of the notable changes to test are:
- Ported almost all of the UI to Pigui
- A significantly more disk efficient save structure
- Commodity prices have been rebalanced
- Added a new ship, the Skipjack Courier
- New and improved lore for 1 Orionis

Please play and test as much as possible, and report any bugs you find.

Have fun and fly safe!



# Pioneer 2020-02-03

Another year, another busy Pioneer development cycle. As always, you can read the full [Changelog](https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt) to see all of the work that's been done, but this time we're listing out some of the major improvements.

## Major changes and new features

- Atmospheric flight - initial implementation of atmospheric lift and drag
- Manual player face generator
- Custom Wolf 359 system added
- Remove rings around Venus and Ariel
- Exposed more game inputs to the new Input system
- Improve "Continue" button
- Many UI fixes and improvements, and moving to pigui:
   - Shipmarket, Save/Load windows, lobby, commodity- &  equipment market, InfoView, Shipinfo, and PersonalView
- Ship warning system also detects missiles
- Reworked and sped up calculations for displaying body / ship names in World View
- Restore features lost in code migration
- Add more station names
- Game start date relative current date
- Make windows installer remove old install
- Music upgrade
- Star rendering optimisation
- Basic Atmospheric heating re-enabled
- Gracefully handle destroyed target ships in SAR missions
- Input/settings system now available in translation system
- Add instructions on building pioneer with CMake and MSYS2
- Use SDL2's AudioDevice APIs for sound handling to reduce likelihood of crashes and unexpected behavior
- Moved Lua's Vector2 / Vector3 implementations to C++ to speed up math-heavy Lua code


## Bug-fixes

- Fix flickering ships in orbit view
- Fix flight UI direction and vector indicators
- Fix crash on main menu
- Fix middle mouse not working in paused mode
- Fix memory-leak in German language
- Fix memory-leak when saving
- Fix calculation of latitude and longitude for PlanetaryInfo
- Fix bug causing segmentation fault
- Fix midhyperjump game crash
- Fix medical  emergency mission
- Fix transparency on menu items
- Fix game load crash with unattached hyperspace clouds
- Fix game load crash typo
- Fix an error preventing keybinding when joysticks are disabled
- Fixes loading old SGM models



# Pioneer 2019-02-03

One year since last build release, tagging and marking this as a release. Please see the [change log]( https://github.com/pioneerspacesim/pioneer/blob/master/Changelog.txt) for what's new, modified, improved, or nerfed. Enjoy!

(Also, alternative linux32 and linux64 download [here](https://flathub.org/apps/details/net.pioneerspacesim.Pioneer))
