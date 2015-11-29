facegen directory structure

Video-link images are built out of a set of layered parts. Each part is
centered horizontally and offset vertically when it's built into the face.

Parts (in the order they're drawn; i.e., bottom layer first):
  - Background   -- There's only one of these.
  - Head
  - Clothes      -- only drawn if not armoured
  - Eyes
  - Nose
  - Mouth
  - Accessories  -- only drawn if not armoured
  - Hair         -- only drawn if not armoured
  - Armour

All parts should be exactly 295 x 285 pixels so that their positions all
match up.

Parts are grouped by species, race and gender.
There can be up to 10 species, and up to 16 races. There should be 2 genders
(I'd like to remove this restriction, but some parts of the code still assume
2 genders).

Accessories and armour are non-race-specific and non-gender-specific.
Clothing is non-race-specific but is gender-specific.
Head, eyes, nose, mouth and hair are race- and gender-specific.

(Technical note: the code is a bit more flexible than this, but the directory
structure imposes this hierarchy.)

Directory structure:

  - backgrounds/general.png  --  The background image.
  - species_$S/
    - accessories/
      - acc_0.png  -- acc_0.png should always just be a blank image
      - acc_$N.png
    - clothes/
      - armour_$N.png
      - cloth_$G_$N.png
    - race_$R/
      - eyes/
        - eyes_$G_$N.png
      - hair/
        - hair_$G_$N.png
      - head/
        - head_$G_$N.png
      - mouth/
        - mouth_$G_$N.png
      - nose/
        - nose_$G_$N.png

In the above, $S is the species index (0 to 9), $R is the race index (0 to 15),
$G is the gender index (0 or 1), $N is the part number. You don't have to
match up the number of items between the genders or between races -- e.g.,
one race could have just one hairstyle for each gender and another race could
have 3 hairstyles for gender 0 and 7 hairstyles for gender 1.

For the eyes, hair, head, mouth and nose, it is possible to create non-gendered
parts by naming them e.g., eyes_$N.png (ie, just put one number in the file name
rather than two). Parts named like that will be assumed to work for any gender.
