import types
from luaparser import Parser


def EncodeVal(x):
    if hasattr(x, 'DumpLua'):
        return '\n'.join(x.DumpLua())
    if isinstance(x, list):
        return '{%s}' % ', '.join([EncodeVal(y) for y in x])
    if isinstance(x, str) and len(x) > 80 and '[[' not in x:
        return '[[%s]]' % x
    return repr(x)


def EncodeVals(*argv):
    res = []
    for x in argv:
        res.append(EncodeVal(x))
    return ', '.join(res)


class LuaObjectFactory:
    def __init__(self, cls, **argv):
        self.cls = cls
        self.context = argv

    def lua_new(self, *args):
        obj = self.cls(*args)
        for k, v in self.context.items():
            setattr(obj, k, v)
        return obj


class LuaObject:
    def __new__(cls, *args, **kwargs):
        obj = super().__new__(cls)
        for name, typ in obj.PROPERTIES:

            def g(name, typ):
                def property_setter(self, val):
                    if not isinstance(val, typ):
                        print("%s type expected for [%s], got %s: %s" %
                              (typ, name, type(val), repr(val)))
                    setattr(self, name, val)
                    return self

                return property_setter

            setattr(obj, "lua_%s" % name, types.MethodType(g(name, typ), obj))
            setattr(obj, name, None)

        return obj

    @classmethod
    def lua_new(cls, *args):
        return cls(*args)


class FixedPoint:
    def __init__(self, numerator, denominator):
        self.numerator = numerator
        self.denominator = denominator

    def to_float(self):
        return self.numerator / self.denominator

    def DumpLua(self):
        return [
            'f(%s, %s)' % (EncodeVal(self.numerator),
                           EncodeVal(self.denominator))
        ]


class Angle:
    def __init__(self, degrees):
        self.degrees = degrees

    def to_float(self):
        return self.degrees

    def DumpLua(self):
        return ['math.deg2rad(%s)' % EncodeVal(self.degrees)]


class FixedAngle:
    def __init__(self, degrees):
        self.degrees = degrees

    def to_float(self):
        return self.degrees.to_float()

    def DumpLua(self):
        return ['fixed.deg2rad(%s)' % EncodeVal(self.degrees)]


class Vector:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def __iter__(self):
        yield self.x
        yield self.y
        yield self.z

    def DumpLua(self):
        return [
            'v(%s, %s, %s)' % tuple(
                [EncodeVal(getattr(self, x)) for x in 'xyz'])
        ]


class CustomSystem(LuaObject):
    PROPERTIES = [
        ('govtype', str),
        ('lawlessness', FixedPoint),
        ('short_desc', str),
        ('long_desc', str),
        ('faction', str),
        ('seed', int),
    ]

    def lua_bodies(self, star, planets):
        self.star = star
        star.SetSattelites(planets)

    def lua_add_to_sector(self, x, y, z, v):
        self.sector_coord = (x, y, z)
        self.in_sector_coord = tuple(v)
        if hasattr(self, 'systems_set'):
            self.systems_set.AddSystem(self)
        return self

    def __init__(self, name, types):
        self.name = name
        self.types = types
        self.star = None

    def DumpLua(self):
        res = []
        res.append(self.__class__.__name__)
        res.append("  :new(%s)" % EncodeVals(self.name, self.types))
        for x, _ in self.PROPERTIES:
            if getattr(self, x) is not None:
                res.append('  :%s(%s)' % (x, EncodeVal(getattr(self, x))))
        if self.star:
            res.append('  :bodies(')
            res.extend(['    ' + x for x in self.star.DumpLua()])
            res[-1] += ')'
        res.append('  :add_to_sector(%s)' % EncodeVals(
            *self.sector_coord, Vector(*self.in_sector_coord)))
        return res


class CustomSystemBody(LuaObject):
    PROPERTIES = [
        ('radius', FixedPoint),
        ('mass', FixedPoint),
        ('temp', int),
        ('seed', int),
        ('semi_major_axis', FixedPoint),
        ('eccentricity', FixedPoint),
        ('inclination', Angle),
        ('rotation_period', FixedPoint),
        ('axial_tilt', FixedAngle),
        ('metallicity', FixedPoint),
        ('volcanicity', FixedPoint),
        ('atmos_density', FixedPoint),
        ('atmos_oxidizing', FixedPoint),
        ('ocean_cover', FixedPoint),
        ('ice_cover', FixedPoint),
        ('life', FixedPoint),
        ('orbital_phase_at_start', FixedAngle),
        ('rotational_phase_at_start', FixedAngle),
        ('latitude', Angle),
        ('longitude', Angle),
        ('space_station_type', str),
        ('orbital_offset', FixedAngle),
        ('equatorial_to_polar_radius', FixedPoint),
    ]

    def lua_rings(self, f1, f2, lst):
        self.rings = {
            'min_radius': f1,
            'max_radius': f2,
            'red': lst[0],
            'green': lst[1],
            'blue': lst[2],
            'alpha': lst[3],
        }
        return self

    def lua_height_map(self, map, index):
        self.height_map = (map, index)
        return self

    def SetSattelites(self, sattelites):
        for x in sattelites:
            if isinstance(x, list):
                self.sattelites[-1].SetSattelites(x)
            else:
                if x.typ == 'STARPORT_SURFACE':
                    self.starports.append(x)
                else:
                    self.sattelites.append(x)

    def __init__(self, name, typ):
        self.name = name
        self.typ = typ
        self.sattelites = []
        self.starports = []
        self.rings = None
        self.height_map = None

    def DumpLua(self):
        res = []
        res.append(self.__class__.__name__)
        res.append("  :new(%s)" % EncodeVals(self.name, self.typ))
        for x, _ in self.PROPERTIES:
            if getattr(self, x) is not None:
                res.append('  :%s(%s)' % (x, EncodeVal(getattr(self, x))))
        if self.height_map:
            res.append("  :height_map(%s)" % EncodeVals(*self.height_map))
        if self.rings:
            res.append("  :rings(%s)" % EncodeVals(self.rings['min_radius'],
                                                   self.rings['max_radius'], [
                                                       self.rings['red'],
                                                       self.rings['green'],
                                                       self.rings['blue'],
                                                       self.rings['alpha'],
                                                   ]))
        sats = self.sattelites + self.starports
        if sats:
            res[-1] += ','
            res.append('  {')
            for x in sats:
                res.extend(['    ' + y for y in x.DumpLua()])
                if x != sats[-1]:
                    res[-1] += ','
            res.append('  }')
        return res


def InitLuaParser(**argv):
    x = Parser()
    x.AddObject(b'CustomSystem', LuaObjectFactory(CustomSystem, **argv))
    x.AddObject(b'CustomSystemBody', CustomSystemBody)
    x.AddObject(b'f', FixedPoint)
    x.AddObject(b'v', Vector)
    x.AddObject(b'math.deg2rad', Angle)
    x.AddObject(b'fixed.deg2rad', FixedAngle)
    return x
