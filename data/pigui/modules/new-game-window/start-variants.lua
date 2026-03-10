local StartVariants = {}
local variantList = {}

function StartVariants.register(variant)
	table.insert(variantList, variant)
end

function StartVariants.list()
	return variantList
end

function StartVariants.item(i)
	return variantList[i]
end

return StartVariants
