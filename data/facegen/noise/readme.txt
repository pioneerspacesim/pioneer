Portrait noise textures
========================

PNG images in this folder are used as optional grain/artifact overlays when
building NPC portrait textures.

How they are applied
--------------------

Each portrait channel is multiplied by the noise channel, then scaled by 2:

  output = portrait * noise * 2

The result is that mid-grey noise (RGB 128, 128, 128) has no effect, while 
values above 128 brighten and values below 128 darken.

Images may be any size; they are tiled across the 295 x 285 face if smaller.

Adding new textures
-------------------

Drop any number of .png files here. 
Use descriptive names (e.g. grain_fine.png, crt_heavy.png).

Artists can author noise in any image editor. Keep the average close to 50%
grey if you want a subtle effect, increase contrast for stronger
brightening/darkening.

When testing a new noise image, you can temporarily delete all the other 
noise images to ensure that yours is the only one in the folder, then the 
game will always pick that one.
