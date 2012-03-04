--generic wrapper function
local building = function(name, modelname, bradius, lodpixels)
	define_model(name, {
		info = {
			 scale = 1,
			 bounding_radius = bradius,
			 lod_pixels = lodpixels,
			 materials = {'wall'},
			 tags = {'city_building'},
		},
		static = function(lod)
			--name, diffuse rgba, spec rgb+intensity, emit rgb
			set_material('wall', 0.7,0.7,0.8,1, .2,.2,.2,5, 0,0,0)
			texture('diffuse.png')
			texture_glow('glow.png')
			use_material('wall')
			--text(modelname, v(0, bradius, 0), v(0,0,1), v(1,0,0), 8)
			if lod == 1 then
				load_obj(modelname .. 'LQ.obj')
			elseif lod == 2 then
				load_obj(modelname .. 'MQ.obj')
			else
				load_obj(modelname .. 'HQ.obj')
			end
		end
	})
end

--define buildings
building('kcity01', 'kbuilding',   30, {5, 60, 100})
building('kcity02', 'kbuilding02', 45, {8, 50, 100})
building('kcity03', 'kbuilding03', 50, {12,70, 100})
