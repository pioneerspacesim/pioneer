CONTENTS_SCHEMA = {
    'order': ['systems'],
    'systems': {
        'type':
            'table',
        'title':
            'Systems List',
        'options': {
            'clickable': True,
        },
        'columns': [
            {
                'title': 'Filename',
                'id': 'filename',
                'format': 'string',
            },
            {
                'title': 'System name',
                'id': 'name',
                'format': 'string',
            },
            {
                'title': ('Sector'),
                'id': ('sector'),
                'subfields': [{
                    'title': x,
                    'id': x,
                    'format': 'int',
                    'selector': ('sector_coord', i),
                } for (i, x) in enumerate('xyz')]
            },
            {
                'title': 'Government',
                'id': 'govtype',
                'format': 'string',
            },
            {
                'title': 'Faction',
                'id': 'faction',
                'format': 'string',
            },
        ],
    }
}

SYSTEM_SCHEMA = {
    'order': ['system', 'bodies'],
    'system': {
        'type': ('fields'),
        'title': ('System Properties'),
        'options': {
            'editable': True,
        },
        'fields': [
            {
                'title': 'Filename',
                'id': 'filename',
                'format': 'string',
            },
            {
                'title': 'System name',
                'id': 'name',
                'format': 'string',
            },
            {
                'title': 'System types',
                'id': 'types',
                'format': 'valarray',
                'item': {
                    'format': ('enum'),
                    'enumvals': [
                        'GRAVPOINT',
                        'BROWN_DWARF',
                        'WHITE_DWARF',
                        'STAR_M',
                        'STAR_K',
                        'STAR_G',
                        'STAR_F',
                        'STAR_A',
                        'STAR_B',
                        'STAR_O',
                        'STAR_M_GIANT',
                        'STAR_K_GIANT',
                        'STAR_G_GIANT',
                        'STAR_F_GIANT',
                        'STAR_A_GIANT',
                        'STAR_B_GIANT',
                        'STAR_O_GIANT',
                        'STAR_M_SUPER_GIANT',
                        'STAR_K_SUPER_GIANT',
                        'STAR_G_SUPER_GIANT',
                        'STAR_F_SUPER_GIANT',
                        'STAR_A_SUPER_GIANT',
                        'STAR_B_SUPER_GIANT',
                        'STAR_O_SUPER_GIANT',
                        'STAR_M_HYPER_GIANT',
                        'STAR_K_HYPER_GIANT',
                        'STAR_G_HYPER_GIANT',
                        'STAR_F_HYPER_GIANT',
                        'STAR_A_HYPER_GIANT',
                        'STAR_B_HYPER_GIANT',
                        'STAR_O_HYPER_GIANT',
                        'STAR_M_WF',
                        'STAR_B_WF',
                        'STAR_O_WF',
                        'STAR_S_BH',
                        'STAR_IM_BH',
                        'STAR_SM_BH',
                    ]
                }
            },
            {
                'title': 'Short description',
                'id': 'short_desc',
                'format': 'string',
            },
            {
                'title': 'Long description',
                'id': 'long_desc',
                'format': 'text',
            },
            {
                'title': ('Government'),
                'id': ('govtype'),
                'format': ('enum'),
                'enumvals': [
                    'NONE',
                    'EARTHCOLONIAL',
                    'EARTHDEMOC',
                    'EMPIRERULE',
                    'CISLIBDEM',
                    'CISSOCDEM',
                    'LIBDEM',
                    'CORPORATE',
                    'SOCDEM',
                    'EARTHMILDICT',
                    'MILDICT1',
                    'MILDICT2',
                    'EMPIREMILDICT',
                    'COMMUNIST',
                    'PLUTOCRATIC',
                    'DISORDER',
                ]
            },
            {
                'title': 'Lawlessness',
                'id': 'lawlessness',
                'format': 'float',
            },
            {
                'title': 'Faction',
                'id': 'faction',
                'format': 'string',
            },
            {
                'title': ('Sector'),
                'id': ('sector'),
                'subfields': [{
                    'title': x,
                    'id': x,
                    'format': 'int',
                    'selector': ('sector_coord', i),
                } for (i, x) in enumerate('xyz')]
            },
            {
                'title': ('Coordinates'),
                'id': ('coord'),
                'subfields': [{
                    'title': x,
                    'id': x,
                    'format': 'float',
                    'selector': ('in_sector_coord', i),
                } for (i, x) in enumerate('xyz')]
            },
        ],
    },
    'bodies': {
        'type':
            'table',
        'title':
            'System Bodies',
        'options': {
            'collapsible': True,
            'editable': True,
        },
        'columns': [
            {
                'title': 'Body name',
                'id': 'name',
                'format': 'string',
            },
            {
                'title': ('Type'),
                'id': ('typ'),
                'format': ('enum'),
                'enumvals': [
                    'GRAVPOINT',
                    'BROWN_DWARF',
                    'WHITE_DWARF',
                    'STAR_M',
                    'STAR_K',
                    'STAR_G',
                    'STAR_F',
                    'STAR_A',
                    'STAR_B',
                    'STAR_O',
                    'STAR_M_GIANT',
                    'STAR_K_GIANT',
                    'STAR_G_GIANT',
                    'STAR_F_GIANT',
                    'STAR_A_GIANT',
                    'STAR_B_GIANT',
                    'STAR_O_GIANT',
                    'STAR_M_SUPER_GIANT',
                    'STAR_K_SUPER_GIANT',
                    'STAR_G_SUPER_GIANT',
                    'STAR_F_SUPER_GIANT',
                    'STAR_A_SUPER_GIANT',
                    'STAR_B_SUPER_GIANT',
                    'STAR_O_SUPER_GIANT',
                    'STAR_M_HYPER_GIANT',
                    'STAR_K_HYPER_GIANT',
                    'STAR_G_HYPER_GIANT',
                    'STAR_F_HYPER_GIANT',
                    'STAR_A_HYPER_GIANT',
                    'STAR_B_HYPER_GIANT',
                    'STAR_O_HYPER_GIANT',
                    'STAR_M_WF',
                    'STAR_B_WF',
                    'STAR_O_WF',
                    'STAR_S_BH',
                    'STAR_IM_BH',
                    'STAR_SM_BH',
                    'PLANET_GAS_GIANT',
                    'PLANET_ASTEROID',
                    'PLANET_TERRESTRIAL',
                    'STARPORT_ORBITAL',
                ]
            },
            {
                'title': 'Radius',
                'id': 'radius',
                'format': 'siprefix',
                'type': 'float',
                'editlabel': 'Relative to Sun/Earth',
            },
            {
                'title': 'Mass',
                'id': 'mass',
                'format': 'siprefix',
                'type': 'float',
                'editlabel': 'Relative to Sun/Earth',
            },
            {
                'title': '🌡',
                'id': 'temp',
                'format': 'int',
                'editsuffix': 'K',
            },
            {
                'title': 'Semi Major Axis',
                'id': 'semi_major_axis',
                'format': 'siprefix',
                'type': 'float',
                'editsuffix': 'au',
            },
            {
                'title': 'Eccent\u200Bricity',
                'id': 'eccentricity',
                'format': 'percent',
                'type': 'float',
            },
            {
                'title': 'Incli\u200Bnation',
                'id': 'inclination',
                'format': 'degrees',
                'type': 'float',
            },
            {
                'title': 'Rotation Period',
                'id': 'rotation_period',
                'format': 'siprefix',
                'type': 'float',
                'editsuffix': 'days',
            },
            {
                'title': 'Axial Tilt',
                'id': 'axial_tilt',
                'format': 'degrees',
                'type': 'float',
            },
            {
                'title': 'Metal\u200Blicity',
                'id': 'metallicity',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Volca\u200Bnicity',
                'id': 'volcanicity',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Atmo\u200Bspheric Density',
                'id': 'atmos_density',
                'format': 'siprefix',
                'type': 'float',
                'editsuffix': 'bar',
            },
            {
                'title': 'Atmo\u200Bspheric Oxidi\u200Bzing',
                'id': 'atmos_oxidizing',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Ocean Cover',
                'id': 'ocean_cover',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Ice Cover',
                'id': 'ice_cover',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Life',
                'id': 'life',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Orbital phase at start',
                'id': 'orbital_phase_at_start',
                'format': 'degrees',
                'type': 'float',
            },
            {
                'title': 'Rota\u200Btional phase at start',
                'id': 'rotational_phase_at_start',
                'format': 'degrees',
                'type': 'float',
            },
            {
                'title': 'Orbi\u200Btal offset',
                'id': 'orbital_offset',
                'format': 'float',
                'type': 'float',
            },
            {
                'title': 'Equa\u200Btorial to polar radius',
                'id': 'equatorial_to_polar_radius',
                'format': 'float',
                'type': 'float',
            },
            {
                'title': 'Space Station type',
                'id': 'space_station_type',
                'format': 'string',
            },
            {
                'title':
                    'Rings',
                'id':
                    'rings',
                'dive':
                    True,
                'subfields': [
                    {
                        'title': 'Min Radius',
                        'id': 'min_radius',
                        'format': 'float',
                    },
                    {
                        'title': 'Max Radius',
                        'id': 'max_radius',
                        'format': 'float',
                    },
                    {
                        'title': 'Red',
                        'id': 'red',
                        'format': 'percent',
                        'type': 'float',
                    },
                    {
                        'title': 'Green',
                        'id': 'green',
                        'format': 'percent',
                        'type': 'float',
                    },
                    {
                        'title': 'Blue',
                        'id': 'blue',
                        'format': 'percent',
                        'type': 'float',
                    },
                    {
                        'title': 'Alpha',
                        'id': 'alpha',
                        'format': 'percent',
                        'type': 'float',
                    },
                ],
            },
            {
                'title':
                    'Height map',
                'id':
                    'height_map',
                'dive':
                    True,
                'subfields': [
                    {
                        'title': 'Map',
                        'id': 'map',
                        'format': 'string',
                    },
                    {
                        'title': 'Index',
                        'id': 'index',
                        'format': 'int',
                    },
                ],
            },
            {
                'title': 'Seed',
                'id': 'seed',
                'format': 'hex',
                'type': 'int',
            },
        ],
        'subsections': [
            {
                'id': 'sattelites',
                'schema': 'bodies',
            },
            {
                'id': 'starports',
                'schema': 'starports',
            },
        ],
    },
    'starports': {
        'type':
            'table',
        'title':
            'Surface starports',
        'options': {
            'collapsible': False,
            'editable': True,
        },
        'columns': [
            {
                'title': 'Body name',
                'id': 'name',
                'format': 'string',
            },
            {
                'title': 'Type',
                'id': 'typ',
                'format': 'enum',
                'enumvals': ['STARPORT_SURFACE'],
            },
            {
                'title': 'Latitude',
                'id': 'latitude',
                'format': 'float',
            },
            {
                'title': 'longitude',
                'id': 'longitude',
                'format': 'float',
            },
        ]
    }
}
