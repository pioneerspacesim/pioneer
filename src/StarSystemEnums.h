// no include guard because this file must be included more than once per translation unit

#ifdef EconType_ITEM
EconType_ITEM(MINING, 1<<0)
EconType_ITEM(AGRICULTURE, 1<<1)
EconType_ITEM(INDUSTRY, 1<<2)
#undef EconType_ITEM
#endif

#ifdef BodyType_ITEM
BodyType_ITEM(GRAVPOINT, 0)
BodyType_ITEM(BROWN_DWARF, 1) //  L+T Class Brown Dwarfs
BodyType_ITEM(WHITE_DWARF, 2)
BodyType_ITEM(STAR_M, 3) //red
BodyType_ITEM(STAR_K, 4) //orange
BodyType_ITEM(STAR_G, 5) //yellow
BodyType_ITEM(STAR_F, 6) //white
BodyType_ITEM(STAR_A, 7) //blue/white
BodyType_ITEM(STAR_B, 8) //blue
BodyType_ITEM(STAR_O, 9)  //blue/purple/white
BodyType_ITEM(STAR_M_GIANT, 10) 
BodyType_ITEM(STAR_K_GIANT, 11) 
BodyType_ITEM(STAR_G_GIANT, 12) 
BodyType_ITEM(STAR_F_GIANT, 13) 
BodyType_ITEM(STAR_A_GIANT, 14) 
BodyType_ITEM(STAR_B_GIANT, 15) 
BodyType_ITEM(STAR_O_GIANT, 16)
BodyType_ITEM(STAR_M_SUPER_GIANT, 17) 
BodyType_ITEM(STAR_K_SUPER_GIANT, 18) 
BodyType_ITEM(STAR_G_SUPER_GIANT, 19)
BodyType_ITEM(STAR_F_SUPER_GIANT, 20)
BodyType_ITEM(STAR_A_SUPER_GIANT, 21) 
BodyType_ITEM(STAR_B_SUPER_GIANT, 22) 
BodyType_ITEM(STAR_O_SUPER_GIANT, 23) 
BodyType_ITEM(STAR_M_HYPER_GIANT, 24) 
BodyType_ITEM(STAR_K_HYPER_GIANT, 25) 
BodyType_ITEM(STAR_G_HYPER_GIANT, 26) 
BodyType_ITEM(STAR_F_HYPER_GIANT, 27) 
BodyType_ITEM(STAR_A_HYPER_GIANT, 28) 
BodyType_ITEM(STAR_B_HYPER_GIANT, 29) 
BodyType_ITEM(STAR_O_HYPER_GIANT, 30) // these various stars do exist, they are transitional states and are rare
BodyType_ITEM(STAR_M_WF, 31)  //Wolf-Rayet star
BodyType_ITEM(STAR_B_WF, 32)  // while you do not specifically get class M,B or O WF stars,
BodyType_ITEM(STAR_O_WF, 33) //  you do get red, blue and purple from the colour of the gasses, so spectral class is an easy way to define them. 
BodyType_ITEM(STAR_S_BH, 34) //stellar blackhole
BodyType_ITEM(STAR_IM_BH, 35) //Intermediate-mass blackhole
BodyType_ITEM(STAR_SM_BH, 36) //Supermassive blackhole
BodyType_ITEM(PLANET_GAS_GIANT, 37)
BodyType_ITEM(PLANET_ASTEROID, 38)
BodyType_ITEM(PLANET_TERRESTRIAL, 39)
BodyType_ITEM(STARPORT_ORBITAL, 40)
BodyType_ITEM(STARPORT_SURFACE, 41)
BodyType_ITEM_X(MIN, TYPE_BROWN_DWARF)
BodyType_ITEM_X(MAX, TYPE_STARPORT_SURFACE)
BodyType_ITEM_X(STAR_MIN, TYPE_BROWN_DWARF)
BodyType_ITEM_X(STAR_MAX, TYPE_STAR_SM_BH)
// XXX need larger atmosphereless thing
#undef BodyType_ITEM
#undef BodyType_ITEM_X
#endif

#ifdef BodySuperType_ITEM
BodySuperType_ITEM(NONE, 0)
BodySuperType_ITEM(STAR, 1)
BodySuperType_ITEM(ROCKY_PLANET, 2)
BodySuperType_ITEM(GAS_GIANT, 3)
BodySuperType_ITEM(STARPORT, 4)
#undef BodySuperType_ITEM
#endif

// no include guards (see note at top)
