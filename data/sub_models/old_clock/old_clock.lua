define_model('clkmin', {
	info = {
			bounding_radius = 5,
			materials={'gold'}
		},
	static = function(lod)
		set_material('gold',.4,.37,.15,1,.4,.35,.1,50,.25,.2,.05)
		use_material('gold')
		texture('antik.png')
		xref_tri(v(0,-.3,.2),v(.3,.17,0),v(0,4.2,.2))
	end
})

define_model('clkhr', {
	info = {
			bounding_radius = 3,
			materials={'gold'}
		},
	static = function(lod)
		set_material('gold',.4,.37,.15,1,.4,.35,.1,50,.25,.2,.05)
		use_material('gold')
		texture('antik.png')
		xref_tri(v(0,-.3,.2),v(.3,.17,0),v(0,3,.2))
	end
})

define_model('old_clock', {
	info = {
			bounding_radius=1,
			lod_pixels = {10,20,50,0},
			materials={'face', 'numbers'}
		},
	static = function(lod)
		set_material('face',.5,.5,.5,1,.3,.3,.3,5)
		set_material('numbers',.3,.27,.15,1,.4,.35,.1,50,.25,.2,.05)
				
		texture('antik.png')
        use_material('numbers')
		zbias(1,v(0,0,0),v(0,0,1))
		text("XII", v(0,4,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("I", v(2,3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("II", v(3.47,2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("III", v(4,0,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("IV", v(3.47,-2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("V", v(2,-3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VI", v(0,-4,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VII", v(-2,-3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VIII", v(-3.47,-2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("IX", v(-4,0,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("X", v(-3.47,2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("XI", v(-2,3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("Potsmoke66 Watches", v(0,-1,0), v(0,0,1), v(1,0,0), 0.3, {center=true})
		text("Switzerland", v(0,-1.5,0), v(0,0,1), v(1,0,0), 0.3, {center=true})
		zbias(0)
		
		use_material('face')
		texture('old_clock.png',v(.496,.495,0),v(.0855,0,0),v(0,.086,0))
		circle(8*lod,v(0,0,0),v(0,0,1),v(0,1,0),5.5)
	end,
	dynamic = function(lod)
		local minutePos = -2*math.pi * get_time('HOURS')
		zbias(3,v(0,0,0),v(0,0,1))
		call_model('clkmin', v(0,0,0),v(math.cos(minutePos),math.sin(minutePos),0),v(math.cos(minutePos+math.pi*0.5), math.sin(minutePos+math.pi*0.5),0), 1)
		local hourPos = minutePos / 12
		zbias(2,v(0,0,0),v(0,0,1))
		call_model('clkhr', v(0,0,0),v(math.cos(hourPos),math.sin(hourPos),0),v(math.cos(hourPos+math.pi*0.5), math.sin(hourPos+math.pi*0.5),0), 1)
	    zbias(0)
	end
})
