-- other jizz

Pi.rand = Rand:new(os.time())

-- Some very useful utility functions --------------------

function _(str, bits)
	-- str = gettext(str)
	s, num = string.gsub(str, '%%([0-9]+)', function(w) return bits[tonumber(w)] end)
	return s
end

function format_money(amount)
	return string.format('$%.1f', amount)
end

Pi.RandomShipRegId = function()
	local letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local a = Pi.rand:Int(1, #letters)
	local b = Pi.rand:Int(1, #letters)
	return string.format("%s%s-%04d", letters:sub(a,a), letters:sub(b,b), Pi.rand:Int(0, 9999))
end
