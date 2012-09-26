-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

function selector1()  -- decal and squad selection
  select1 = util.hash_random(string.sub(get_label(), 4,7), 1, 1000)
end

function selector2() -- skin color selection
  select2 = util.hash_random(string.sub(get_label(), 3,6), 1,100)
end

function selector3() -- dress color selection
  select3 = util.hash_random(string.sub(get_label(), 2,5), 1,100)
end

function selector4() -- female / male selection
  select4 = util.hash_random(string.sub(get_label(), 1,4), 1,100)
end
