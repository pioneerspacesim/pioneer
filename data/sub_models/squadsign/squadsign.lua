--[[
please keep the zbias set to 6
because you can't know what's ahead, but this will cover most without a "printthrough" that happens sometimes with higher values.
model & texture, gernot
--]]



-- this is used if you only like to get the "squad sign" (flightgroup colors) materials and replaces the "squad_color" model which didn't works no more. it's used also in the scripts below.

function squad_color(self)
	-- note: alpha value is set to .99; this is deliberate
	--  If it goes above .99, then LMR switches alpha-blending off,
	--  even though the texture itself has alpha.
	--  The result is that instead of getting a texture-masked decal,
	--  you get a solid square of the material colour.

	selector1()
	if select1 < 201 then
		set_material('squad', .5,0,0,.99,.6,.6,.6,30)
	elseif select1 < 401 then
		set_material('squad', .45,.35,.01,.99,.6,.6,.6,30)
	elseif select1 < 601 then
		set_material('squad', 0,.15,.7,.99,.6,.6,.6,30)
	elseif select1 < 801 then
		set_material('squad', .06,.35,0,.99,.6,.6,.6,30)
	elseif select1 > 800 then
		set_material('squad', .2,0,.35,.99,.6,.6,.6,30)
	end
end


-- has no more dynamic used texture & geometry ;)
define_model('squadsign_1', {
	info =	{
			bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad_1.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end,
	dynamic = function(lod)
		squad_color()
	end
})


--i imported the additional old squadsign variations, even if they are rarely used ("Lanner" only, as far as i know)

define_model('squadsign_2', {
	info =	{
			bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad_2.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end,
	dynamic = function(lod)
		squad_color()
	end
})

define_model('squadsign_3', {
	info =	{
			bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad_3.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end,
	dynamic = function(lod)
		squad_color()
	end
})

define_model('squadsign_4', {
	info =	{
			bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad_4.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end,
	dynamic = function(lod)
		squad_color()
	end
})

-- these are some new "imperial" signs, sorry no material change, i had to exchange the textures for each color, mainly the courier and some other new models will use this.

define_model('imp_sign_a', {
	info =	{
			bounding_radius = 1,
			materials = {'mat'},
			},
	static = function(lod)
		set_material('mat',.7,.68,.65,.99,.2,.2,.2,10)
		use_material('mat')
		texture('impsign_0.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end
})

define_model('imp_sign_b', {
	info =	{
			bounding_radius = 1,
			materials = {'mat'},
			},
	static = function(lod)
		set_material('mat',.7,.68,.65,.99,.2,.2,.2,10)
		use_material('mat')
		texture('impsign_1.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end
})

define_model('imp_sign_c', {
	info =	{
			bounding_radius = 1,
			materials = {'mat'},
			},
	static = function(lod)
		set_material('mat',.7,.68,.65,.99,.2,.2,.2,10)
		use_material('mat')
		texture('impsign_2.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end
})

define_model('imp_sign_d', {
	info =	{
			bounding_radius = 1,
			materials = {'mat'},
			},
	static = function(lod)
		set_material('mat',.7,.68,.65,.99,.2,.2,.2,10)
		use_material('mat')
		texture('impsign_3.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end
})

define_model('imp_sign_e', {
	info =	{
			bounding_radius = 1,
			materials = {'mat'},
			},
	static = function(lod)
		set_material('mat',.7,.68,.65,.99,.2,.2,.2,10)
		use_material('mat')
		texture('impsign_4.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		zbias(6,v(0,0,0),v(-1,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
		zbias(0)
	end
})


-- final "imperial sign" sub-model, will result in the same colors as the squadsign selection.
define_model('imp_sign_1', {
	info =	{
			bounding_radius = 1,
			},
	static = function(lod)
	end,
	dynamic = function(lod)
		selector1()
		if select1 < 201 then
			call_model('imp_sign_a',v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif select1 < 401 then
			call_model('imp_sign_b',v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif select1 < 601 then
			call_model('imp_sign_c',v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif select1 < 801 then
			call_model('imp_sign_d',v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif select1 > 800 then
			call_model('imp_sign_e',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})
