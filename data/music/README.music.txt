Drop music files here. They must be in .ogg format, 44100 or 22050 Hz.

Music in core/ will be played by the default player script,
modules/MusicPlayer.lua. Mission or per-system scripts should add their
own music to a separate subdir.

Music in core/ is selected based on game events. When a particular game event
occurs a song is chosen at random from the appropriate subdir. The subdirs are
as follows:

 - space: ambient music, played while flying in space around the populated
   regions of the galaxy. When music for a specific event finishes the player
   will start a song from this category. This is also the fallback category if
   any category subdir has no songs in it.

 - unexplored: ambient music, played while flying in space far away from
   the populated regions of the galaxy. This is also the fallback category if
   current music ends while player is in hyperspace.

 - menu: main menu theme.

 - discovery: plays if the player jumps to a rare unexplored stellar object.

 - map-core, map-unexplored: plays if the ambient music ends while the player
   is in sector or system map. map-core if player is in the populated regions
   of the galaxy, map-unexplored if far away from civilization.

 - near-planet, near-spacestation: ambient music specific to planets and
   orbital stations. The player will play something from one of these
   categories if ambient music is requested and the player is near a planet or
   station.

 - docked: played when the player docks (ie as the station menu appears)

 - undocked: played when the player undocks (ie when the request launch)

 - ship-nearby, ship-firing: played when the player's alert state changes
   because a ship is nearby (within 100km) or starts firing. Note that the
   presence of a ship (even a firing ship) does not necessarily mean the ship
   is hostile and the player is (about to be) in combat.

 - ship-destroyed: played with the player destroys a ship. Disabled with
   https://github.com/pioneerspacesim/pioneer/pull/5093.

 - player-destroyed: played when the player is destroyed. Will continue to
   play over the "tombstone" screen.
