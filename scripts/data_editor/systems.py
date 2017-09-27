import os
from systemsformat import InitLuaParser


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
    schema = [{
        'type': ('table'),
        'id': ('systems'),
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
                'title':
                'Sector',
                'id':
                'sector',
                'subfields': [{
                    'title': x,
                    'id': x,
                    'format': 'int'
                } for x in 'xyz']
            },
        ],
    }]
    data = []
    for x in self.systems:
      data.append({
          'filename': x.filename,
          'name': x.name,
          'sector': dict(zip('xyz', x.sector_coord)),
      })
    return {'data': {'systems': data}, 'schema': schema}

  def __str__(self):
    return "%d systems: %s" % (
        len(self.systems),
        ', '.join(['%s:%s' % (x.filename, x.name) for x in self.systems]))


def GetSystemsSet():
  x = SystemsSet()
  x.LoadFromDir('../../data/systems/')
  return x
