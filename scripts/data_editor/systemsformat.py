import types
from luaparser import Parser


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
            print("%s type expected for [%s], got %s" % (typ, name, type(val)))
            # raise TypeError("%s type expected for [%s], got %s" % (typ, name,
            #                                                        type(val)))
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


class Angle:
  def __init__(self, radians):
    self.radians = radians


class FixedAngle:
  def __init__(self, radians):
    self.radians = radians


class Vector:
  def __init__(self, x, y, z):
    self.x = x
    self.y = y
    self.z = z

  def __iter__(self):
    yield self.x
    yield self.y
    yield self.z


class CustomSystem(LuaObject):
  PROPERTIES = [
      ('govtype', str),
      ('lawlessness', FixedPoint),
      ('short_desc', str),
      ('long_desc', str),
      ('faction', str),
      ('seed', int),
  ]

  def lua_bodies(self, *lst):
    return self

  def lua_add_to_sector(self, x, y, z, v):
    self.sector_coord = (x, y, z)
    self.in_sector_coord = tuple(v)
    if hasattr(self, 'systems_set'):
      self.systems_set.AddSystem(self)
    return self

  def __init__(self, name, types):
    self.name = name
    self.types = types


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
    return self

  def lua_height_map(self, map, index):
    self.height_map = (map, index)
    return self

  def __init__(self, name, typ):
    self.name = name
    self.typ = typ


def InitLuaParser(**argv):
  x = Parser()
  x.AddObject(b'CustomSystem', LuaObjectFactory(CustomSystem, **argv))
  x.AddObject(b'CustomSystemBody', CustomSystemBody)
  x.AddObject(b'f', FixedPoint)
  x.AddObject(b'v', Vector)
  x.AddObject(b'math.deg2rad', Angle)
  x.AddObject(b'fixed.deg2rad', FixedAngle)
  return x
