-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

function gelati(pos,rot,scale)  -- pos=position, rot=rotation value, scale=scale x,y,z,
	use_material('pine')
	texture('pine_01.png')
	load_obj('pine_01.obj', matrix.translate(pos) * matrix.rotate(rot*math.pi,v(0,1,0)) * matrix.scale(scale))
end

function broccoli(pos,rot,scale) -- pos=position, rot=rotation value, scale=scale x,y,z,
    use_material('oak')
	texture('oak_01.png')
	load_obj('oak_01.obj', matrix.translate(pos) * matrix.rotate(rot*math.pi,v(0,1,0)) * matrix.scale(scale))
end

define_model('woods_0', {

	info = {
	        bounding_radius = 30,
	        materials = {'pine', 'oak'},
	        },

	static = function(lod)
        set_material('oak', .6,.6,.6, .99, .4,.4,.3, 1)
        set_material('pine', .5,.5,1, .99, .5,.4,.3, 1)

		--[[
		sizes:
		normal = 1:1
		large  = 3:2
		wide   = 2:3
		--]]

        gelati(v(10,0,15),0,v(20,40,20))
		gelati(v(-15,0,20),.3,v(17,25,17)) -- large
        broccoli(v(20,0,5),0,v(15,10,15)) -- wide
		gelati(v(15,0,-15),1.5,v(9,6,9))
		broccoli(v(-20,0,-5),1.2,v(10,30,10))
		gelati(v(10,0,-10),1,v(12,12,12)) -- normal
		broccoli(v(-5,0,-20),1.2,v(9,9,9))
		broccoli(v(5,0,-30),.7,v(9,9,9))
		broccoli(v(0,0,0),.4,v(15,15,15))
		gelati(v(20,0,-30),1,v(15,10,15))
		broccoli(v(35,0,20),.3,v(10,30,10))
		broccoli(v(-15,0,-35),1.5,v(20,13,20))
		gelati(v(-30,0,-20),1.7,v(20,30,20))
		broccoli(v(-45,0,-10),.2,v(15,10,15))
		gelati(v(-15,0,5),.6,v(20,15,20))
		broccoli(v(-35,0,10),.7,v(15,30,15))
		gelati(v(-5,0,25),.5,v(20,10,20))
	end
})

define_model('woods_1', {

	info = {
	        bounding_radius = 30,
         	materials = {'pine', 'oak'},
	        },

	static = function(lod)
     	set_material('oak', .6,.6,.6, .99, .4,.4,.3, 1)
        set_material('pine', .5,.5,1, .99, .5,.4,.3, 1)

		gelati(v(10,0,15),0,v(20,30,20)) -- large
		gelati(v(-15,0,20),.3,v(17,25,17))
		broccoli(v(20,0,5),0,v(15,10,15)) -- wide
		gelati(v(15,0,-15),1.5,v(9,6,9))
		broccoli(v(-20,0,-5),.6,v(10,30,10))
		gelati(v(10,0,-10),1,v(12,12,12)) -- normal
		broccoli(v(-5,0,-20),1.2,v(6,6,6))
		broccoli(v(5,0,-20),.7,v(6,6,6))
	end
})

