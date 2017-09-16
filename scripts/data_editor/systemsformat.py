import types
from luaparser import Parser
import os


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


# For debug
if __name__ == "__main__":
  for root, dirs, files in os.walk('../../data/systems'):
    for file in files:
      if not file.endswith('.lua'):
        continue
      f = os.path.join(root, file)
      print(f)
      x = Parser()
      x.AddObject(b'CustomSystem', CustomSystem)
      x.AddObject(b'CustomSystemBody', CustomSystemBody)
      x.AddObject(b'f', FixedPoint)
      x.AddObject(b'v', Vector)
      x.AddObject(b'math.deg2rad', Angle)
      x.AddObject(b'fixed.deg2rad', FixedAngle)
      x.Parse(f)