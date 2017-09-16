from lualexer import TokenStream


class Parser:
  def __init__(self):
    self.dict = {}

  def AddObject(self, name, obj):
    self.dict[name] = obj

  def Parse(self, filename):
    ts = TokenStream(filename)

    while not ts.Peek().IsEof():
      self.ParseStatement(ts)

  def ParseStatement(self, ts):
    variable_name = None
    if ts.Peek().IsWord(b'local'):
      ts.Next()  # local
      variable_name = ts.Next().WordVal()
      ts.Next().ExpectPunct(b'=')
    obj = self.ParseObject(ts)
    if variable_name:
      self.dict[variable_name] = obj

  # Only supports expressions only with multiplication. :)  e.g. 24*60*3
  def ParseLiteralExpr(self, ts):
    v = ts.Next().LiteralVal()
    while ts.Peek().IsPunct(b'*'):
      ts.Next()  # '*'
      v *= ts.Next().LiteralVal()

    return v

  def ParseObject(self, ts):
    if ts.Peek().IsPunct(b'{'):
      ts.Next()  # '{'
      lst = self.ParseList(ts)
      ts.Next().ExpectPunct(b'}')
      return lst

    if ts.Peek().IsLiteral():
      return self.ParseLiteralExpr(ts)

    ctx = self.dict[ts.Next().WordVal()]

    # Parse chain of method calls.
    while True:
      if ts.Peek().IsPunct(b':'):
        ts.Next()  # :
        method = ts.Next().WordVal()
        ctx = getattr(ctx, 'lua_%s' % method.decode('utf-8'))
      elif ts.Peek().IsPunct(b'('):
        ts.Next()  # (
        lst = self.ParseList(ts)
        ts.Next().ExpectPunct(b')')
        ctx = ctx(*lst)
      else:
        break

    return ctx

  def ParseList(self, ts):
    res = []
    while True:
      t = ts.Peek()
      if not (t.IsLiteral() or t.IsPunct(b'{') or t.IsWord()):
        return res
      res.append(self.ParseObject(ts))
      if ts.Peek().IsPunct(b','):
        ts.Next()
      else:
        return res