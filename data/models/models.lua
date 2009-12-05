math.clamp = function(v, min, max)
	return math.min(max, math.max(v,min))
end

function lerp_materials(a, m1, m2)
	local out = {}
	a = math.fmod(a, 2.002)
	if a > 1.0 then a = 2.002 - a end
	local b = 1.0 - a
	for i = 1,11 do
		out[i] = a*m2[i] + b*m1[i]
	end
	return out
end

dofile "data/models/ships.lua"
dofile "data/models/city.lua"

function test_info()
	return { lod_pixels={30,60,100,0},
		bounding_radius = 10.0,
		tags = {'building', 'turd'},
		materials = {'red', 'shinyred'}
	}
end
function test_static(lod)
	set_material("red", 1,0,0,1)
	set_material("shinyred", 1,0,0,1, 1,1,1,50)
	use_material("red")
	xref_flat(16, v(0,0,1),
		{v(4,0,0)}, -- straight line bit
		{v(4.5,-0.5,0),v(5,0,0)}, -- quadric bezier bit
		{v(5,0.5,0),v(4,0,0),v(4,1,0)}, -- cubic bezier bit
		{v(3,0.5,0)}, -- straight line bit
		{v(3,0.3,0)} -- etc
		)
	zbias(1, v(0,5,0), v(0,0,1))
	geomflag(0x8000)
	text("LOD: " .. tostring(lod), v(0,5,0), v(0,0,1), v(1,1,0):norm(), 1.0)
	geomflag(0)
	zbias(0)
	use_material("red")
	xref_cylinder(lod*4, v(5,0,0), v(10,0,0), v(0,1,0), 1.0)
	use_material("shinyred")
	xref_circle(9, v(4,5,0), v(0,0,1), v(1,0,0), 1.0)
	tri(v(12,3,0),v(13,3,0), v(12,4,0))
	xref_tri(v(13,3,0),v(14,3,0), v(13,4,0))
	xref_quad(v(6,6,0), v(7,6,0), v(7,7,0),v(6,7,0))
--[[
	xref_cubic_bezier_tri(32, v(0,0,0), v(1,0,0), v(2,0,0), v(3,0,0),
				v(0,-1,0), v(1,-1,3), v(3,-1,0),
				v(0,-2,0), v(2,-2,0),
				v(0,-3,0))
				--]]
---[[
	xref_quadric_bezier_tri(16, v(0,0,0), v(1,0,0), v(2,0,0),
				v(0,-1,0), v(1,-1,2),
				v(0,-2,0))
				--]]
--[[	
	geomflag(0x8000)
	xref_quadric_bezier_quad(16, 16,
			v(0,0,0), v(1,-1,0), v(2,0,0),
			v(-1,1,0), v(1,1,8), v(3,1,0),
			v(0,2,0), v(1,3,0), v(2,2,0))
			--]]
--[[	
	xref_cubic_bezier_quad(32, 32,
			v(0,0,0), v(1,0,0), v(2,0,0), v(3,0,0),
			v(0,1,0), v(1,1,5), v(2,1,0), v(3,1,0),
			v(0,2,0), v(1,2,0), v(2,2,0), v(3,2,0),
			v(0,4,0), v(1,4,0), v(1,4,0), v(1,3,0))
			--]]
--[[
	extrusion(v(0,0,0), v(0,0,-5), v(0,1,0), 1.0,
		v(1,0,0), v(0.5, 0.8, 0), v(0,1,0), v(-0.5,0.8,0), v(-1,0,0),
		v(0,-0.5,0))
		--]]
end
poo = 0
function test_dynamic(lod)
	poo = poo + 0.005
	set_material("red", math.sin(poo)*math.sin(poo), 0.5, 0.5, 1)
	use_material("red")
	xref_cylinder(16, v(-8,0,0), v(-8,5,0), v(1,0,0), math.abs(math.sin(poo)))
	circle(9, v(5*math.sin(poo),5*math.cos(poo),0), v(0,0,1), v(1,0,0), 1.0)

	local ang = 2*math.pi*get_arg(0)
	--call_model("blob", v(0,0,-20), v(1,0,0), v(1,1,0),1.0)
end

function blob_info()
	return {
		bounding_radius=8,
		materials={'blue'}
	}
end
function blob_static(lod)
	set_material("blue", 0,0,1,1)
	use_material("blue")
	cylinder(16, v(-5,0,0), v(-5,5,0), v(1,0,0), 1.0)
	text("blob_static()", v(-5,-2,0), v(0,0,1), v(1,0,0), 0.5)
	xref_thruster(v(5,0,0), v(0,0,-1), 10)
	xref_thruster(v(5,0,0), v(0,0,1), 10)
	xref_thruster(v(5,0,0), v(0,1,0), 5)
	xref_thruster(v(5,0,0), v(0,-1,0), 5)
	thruster(v(5,0,-5), v(1,0,0), 5, true)
	thruster(v(5,0,5), v(1,0,0), 5, true)
	thruster(v(-5,0,-5), v(-1,0,0), 5, true)
	thruster(v(-5,0,5), v(-1,0,0), 5, true)
	text("HELLO FROM BLOB", v(0,0,0), v(0,0,1), v(1,0,0), 10.0)
end

m = Mat4x4.rotate(math.pi*0.25,v(1,1,1))
m:print()
m = m:inverse()
m:print()
a = (m*v(1,0,0))
a:print()

function cargo_info()
	return {
		lod_pixels = {20, 50, 0},
		bounding_radius = 1.5,
		materials = {'body', 'text'}
	}
end
function cargo_static(lod)
	local divs = 8*lod
	set_material('body', .5,.5,.5,1, 0,0,0, 0, 0,0,0)
	set_material('text', 1,0,0,1, 0,0,0, 0, 0,0,0)
	local top = v(0,1,0)
	local bottom = v(0,-1,0)
	use_material('body')
	cylinder(divs, top, bottom, v(1,0,0), 1.0)
end
function cargo_dynamic(lod)
	if lod == 3 then
		local textpos = v(0,1,0)
		use_material('text')
		zbias(1, textpos, v(0,1,0))
		text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 0.1, {center=true})
	end
end

register_models("blob","test","cargo")
