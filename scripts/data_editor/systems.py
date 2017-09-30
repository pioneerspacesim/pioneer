import os
from systemsformat import InitLuaParser

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
    'order': ['system'],
    'system': {
        'type': ('fields'),
        'title': ('System Properties'),
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
    }
}


def PopulateFromSchema(data, schema):
  res = {}
  for x in schema:
    if 'subfields' in x:
      res[x['id']] = PopulateFromSchema(data, x['subfields'])
      continue
    selector = x.get('selector', x['id'])
    if not isinstance(selector, tuple):
      selector = (selector, )
    d = data
    for term in selector:
      if isinstance(term, str) and hasattr(d, term):
        d = getattr(d, term)
      else:
        d = d[term]
    if hasattr(d, 'as_%s' % x['format']):
      d = getattr(d, 'as_%s' % x['format'])()
    res[x['id']] = d
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
    data = []
    for x in self.systems:
      data.append(PopulateFromSchema(x, CONTENTS_SCHEMA['systems']['columns']))
    return {'data': {'systems': data}, 'schema': CONTENTS_SCHEMA}

  def GetSystem(self, file, system):
    for x in self.systems:
      if x.filename == file and x.name == system:
        break
    else:
      raise KeyError

    props = PopulateFromSchema(x, SYSTEM_SCHEMA['system']['fields'])

    return {'data': {'system': props}, 'schema': SYSTEM_SCHEMA}

  def __str__(self):
    return "%d systems: %s" % (
        len(self.systems),
        ', '.join(['%s:%s' % (x.filename, x.name) for x in self.systems]))


def GetSystemsSet():
  x = SystemsSet()
  x.LoadFromDir('../../data/systems/')
  return x
