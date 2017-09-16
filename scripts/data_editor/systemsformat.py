from luaparser import Parser


class LuaObject:
  @classmethod
  def lua_new(cls, *args):
    return cls(*args)


class CustomSystem(LuaObject):
  def __init__(self):
    pass


# For debug
if __name__ == "__main__":
  x = Parser()
  x.AddObject(b'CustomSystem', CustomSystem)
  x.Parse('../../data/systems/00_sol.lua')