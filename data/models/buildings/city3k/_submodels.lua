--[[
--vertical base with a glowing fence
function bld_base_1(lod,scale)
	if lod == 1 then return end

	local v0 = v(3,0,10)*scale
	local v1 = v(-3,0,10)*scale
	local v2 = v(7,0,3)*scale
	local v4 = v(7,0,-3)*scale
	local v6 = v(3,0,-7)*scale
	local v7 = v(-3,0,-7)*scale
	local v10 = v(3,-50,10)*scale
	local v11 = v(-3,-50,10)*scale
	local v12 = v(7,-50,3)*scale
	local v14 = v(7,-50,-3)*scale
	local v16 = v(3,-50,-7)*scale
	local v17 = v(-3,-50,-7)*scale

	local v20 = v(3,1,10)*scale
	local v21 = v(-3,1,10)*scale
	local v22 = v(7,1,3)*scale
	local v24 = v(7,1,-3)*scale
	local v26 = v(3,1,-7)*scale
	local v27 = v(-3,1,-7)*scale

	if lod > 1 then
		use_material('concrete')
		texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,0,1)/scale)
	end
	xref_quad(v0,v2,v4,v6)
	quad(v0,v6,v7,v1)

	if lod > 1 then
		texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,.05,0)/scale)
	end
	quad(v0,v1,v11,v10)
	quad(v6,v16,v17,v7)

	if lod > 1 then
		texture('conc.png', v(.5,.5,0),v(0,0,1/scale),v(0,.05,0)/scale)
	end
	xref_quad(v0,v10,v12,v2)
	xref_quad(v2,v12,v14,v4)
	xref_quad(v4,v14,v16,v6)

	if lod > 1 then
		use_material('fce_glow')

		texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		quad(v20,v0,v1,v21)
		quad(v26,v27,v7,v6)

		texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		xref_quad(v0,v20,v22,v2)
		xref_quad(v2,v22,v24,v4)
		xref_quad(v4,v24,v26,v6)

		texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		quad(v20,v21,v1,v0)
		quad(v26,v6,v7,v27)

		texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		xref_quad(v0,v2,v22,v20)
		xref_quad(v2,v4,v24,v22)
		xref_quad(v4,v6,v26,v24)
	end
end

function bld_base_2(lod,scale)
	local v0 = v(3,0,7)*scale
	local v1 = v(-3,0,7)*scale
	local v2 = v(7,0,3)*scale
	local v4 = v(7,0,-3)*scale
	local v6 = v(3,0,-7)*scale
	local v7 = v(-3,0,-7)*scale
	local v10 = v(3,-40,7)*scale
	local v11 = v(-3,-40,7)*scale
	local v12 = v(7,-40,3)*scale
	local v14 = v(7,-40,-3)*scale
	local v16 = v(3,-40,-7)*scale
	local v17 = v(-3,-40,-7)*scale

	local v20 = v(3,1,7)*scale
	local v21 = v(-3,1,7)*scale
	local v22 = v(7,1,3)*scale
	local v24 = v(7,1,-3)*scale
	local v26 = v(3,1,-7)*scale
	local v27 = v(-3,1,-7)*scale

	if lod > 1 then
	   	use_material('concrete')
		texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,0,1)/scale)
	end
	xref_quad(v0,v2,v4,v6)
	quad(v0,v6,v7,v1)

	if lod > 1 then
		texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,.05,0)/scale)
	end
	quad(v0,v1,v11,v10)
	quad(v6,v16,v17,v7)

	if lod > 1 then
		texture('conc.png', v(.5,.5,0),v(0,0,1)/scale,v(0,.05,0)/scale)
	end
	xref_quad(v0,v10,v12,v2)
	xref_quad(v2,v12,v14,v4)
	xref_quad(v4,v14,v16,v6)

	if lod > 1 then
		use_material('fce_glow')
		texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		quad(v20,v0,v1,v21)
		quad(v26,v27,v7,v6)

		texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		xref_quad(v0,v20,v22,v2)
		xref_quad(v2,v22,v24,v4)
		xref_quad(v4,v24,v26,v6)

		texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		quad(v20,v21,v1,v0)
		quad(v26,v6,v7,v27)

		texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		xref_quad(v0,v2,v22,v20)
		xref_quad(v2,v4,v24,v22)
		xref_quad(v4,v6,v26,v24)
	end
end
--]]
