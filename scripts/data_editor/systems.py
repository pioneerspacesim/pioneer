import os
from systemsformat import InitLuaParser


class UnknownSystem(Exception):
    pass


CONTENTS_SCHEMA = {
    'order': ['systems'],
    'systems': {
        'type': ('table'),
        'title': ('Systems List'),
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
                'format': 'array_string',
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
                'title': 'Government',
                'id': 'govtype',
                'format': 'string',
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
        'type': ('table'),
        'title': ('System Bodies'),
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
                'title': 'Type',
                'id': 'typ',
                'format': 'string',
            },
            {
                'title': 'Radius',
                'id': 'radius',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': 'Mass',
                'id': 'mass',
                'format': 'siprefix',
                'type': 'float',
            },
            {
                'title': '🌡',
                'id': 'temp',
                'format': 'int',
            },
            {
                'title': 'Semi Major Axis',
                'id': 'semi_major_axis',
                'format': 'siprefix',
                'type': 'float',
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
                'title': 'Seed',
                'id': 'seed',
                'format': 'hex',
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
        'type': ('table'),
        'title': ('Surface starports'),
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
                'format': 'string',
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


def FollowSelector(data, schema):
    selector = schema.get('selector', schema['id'])
    if not isinstance(selector, tuple):
        selector = (selector, )
    for term in selector:
        if isinstance(term, str) and hasattr(data, term):
            data = getattr(data, term)
        else:
            data = data[term]
    return data


def PopulateFieldsFromSchema(data, schema):
    res = {}
    for x in schema:
        if 'subfields' in x:
            res[x['id']] = PopulateFieldsFromSchema(data, x['subfields'])
            continue
        d = FollowSelector(data, x)
        typ = x.get('type', x['format'])
        if hasattr(d, 'to_%s' % typ):
            d = getattr(d, 'to_%s' % typ)()
        res[x['id']] = d
    return res


def PopulateFromSchema(data, root_schema, section):
    schema = root_schema[section]
    if 'columns' in schema and isinstance(data, list):
        res = []
        for x in data:
            res.append(PopulateFromSchema(x, root_schema, section))
        return res
    if data is None:
        return None
    res = {}
    if 'columns' in schema:
        res.update(PopulateFieldsFromSchema(data, schema['columns']))
    if 'fields' in schema:
        res.update(PopulateFieldsFromSchema(data, schema['fields']))
    if 'subsections' in schema:
        for x in schema['subsections']:
            d = FollowSelector(data, x)
            res[x['id']] = PopulateFromSchema(d, root_schema, x['schema'])
    return res


class SystemsSet:
    def __init__(self):
        self.systems = []

    def AddSystem(self, system):
        self.systems.append(system)

    def LoadFromDir(self, rootdir):
        for root, dirs, files in os.walk(rootdir):
            for file in sorted(files):
                if not file.endswith('.lua'):
                    continue
                f = os.path.join(root, file)
                filename = f[len(rootdir):]
                print(filename)
                x = InitLuaParser(filename=filename, systems_set=self)
                x.Parse(f)

    def GetContents(self):
        data = PopulateFromSchema(self.systems, CONTENTS_SCHEMA, 'systems')
        return {'data': {'systems': data}, 'schema': CONTENTS_SCHEMA}

    def GetSystem(self, file, system):
        for x in self.systems:
            if x.filename == file and x.name == system:
                break
        else:
            raise UnknownSystem

        sysprops = PopulateFromSchema(x, SYSTEM_SCHEMA, 'system')
        root_body = None
        if x.star:
            root_body = PopulateFromSchema([x.star], SYSTEM_SCHEMA, 'bodies')

        return {
            'data': {
                'system': sysprops,
                'bodies': root_body,
            },
            'schema': SYSTEM_SCHEMA
        }

    def __str__(self):
        return "%d systems: %s" % (
            len(self.systems),
            ', '.join(['%s:%s' % (x.filename, x.name) for x in self.systems]))


def GetSystemsSet():
    x = SystemsSet()
    x.LoadFromDir('../../data/systems/')
    return x
