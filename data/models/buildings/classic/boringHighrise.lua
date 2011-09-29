--[[
define_model('boringHighrise', {
	info = {
		bounding_radius=200,
		materials={'mat1','windows'},
		tags = {'city_building'},
		lod_pixels = {5,30,100},
	},
	static = function(lod)
		set_material("mat1", 0.5,0.5,0.5,1)
		set_material("windows", 0,0,0,1, 1,1,1,50, .5,.5,0)
		use_material("mat1")
		extrusion(v(0,0,20), v(0,0,-20), v(0,1,0), 1.0,
				v(-20,0,0), v(20,0,0), v(20,200,0), v(-20,200,0))
		if lod == 2 then
			local top = 190
			local bot = 10
			local left = -12
			local right = 12
			use_material("windows")
			zbias(1, v(0,0,20), v(0,0,1))
			quad(v(left,bot,20), v(right,bot,20), v(right,top,20), v(left,top,20))
			zbias(1, v(-20,0,0), v(-1,0,0))
			quad(v(-20,bot,left), v(-20,bot,right), v(-20,top,right), v(-20,top,left))
			zbias(1, v(0,0,-20), v(0,0,-1))
			quad(v(20,top,left), v(20,top,right), v(20,bot,right), v(20,bot,left))
			zbias(1, v(0,0,-20), v(0,0,-1))
			quad(v(left,top,-20), v(right,top,-20), v(right,bot,-20), v(left,bot,-20))
			zbias(0)
		end
		if lod > 2 then
			use_material("windows")
			zbias(1, v(0,0,20), v(0,0,1))
			for y = 4,198,5 do
				for x = -17,16,4 do
					quad(v(x,y,20), v(x+2,y,20), v(x+2,y+3,20), v(x,y+3,20))
				end
			end
			zbias(1, v(-20,0,0), v(-1,0,0))
			for y = 4,198,5 do
				for x = -17,16,4 do
					quad(v(-20,y,x), v(-20,y,x+2), v(-20,y+3,x+2), v(-20,y+3,x))
				end
			end
			zbias(1, v(20,0,0), v(1,0,0))
			for y = 4,198,5 do
				for x = -17,16,4 do
					quad(v(20,y+3,x), v(20,y+3,x+2), v(20,y,x+2), v(20,y,x))
				end
			end
			zbias(1, v(0,0,-20), v(0,0,-1))
			for y = 4,198,5 do
				for x = -15,17,4 do
					quad(v(-x,y+3,-20), v(-x+2,y+3,-20), v(-x+2,y,-20), v(-x,y,-20))
				end
			end
			zbias(0)
		end
	end
})
--]]
