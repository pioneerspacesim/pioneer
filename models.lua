
function building1_static()
	dec_material("mat")
	set_material("mat", .5, .5, .5, 1)
	use_material("mat")
	extrusion(v(0,0,-7.5), v(0,0,7.5), v(0,1,0), 6.0,
		v(-1,0,0), v(1,0,0), v(1,1,0), v(-1,1,0))
end

function building2_static()
	dec_material("mat")
	set_material("mat", .5, .5, .5, 1)
	use_material("mat")
	extrusion(v(0,0,-16), v(0,0,16), v(0,1,0), 1.0,
		v(-16,0,0), v(16,0,0), v(16,20,0), v(-16,20,0))
end

function skyscraper1_static()
	-- Inset black bit
	-- h g
	--
	-- e f
	-- d c
	-- a b
	local a = v(-20,0,10)
	local b = v(20,0,10)
	local c = v(20,40,10)
	local d = v(-20,40,10)
	local e = v(-20,48,10)
	local f = v(20,48,10)
	local g = v(20,200,10)
	local h = v(-20,200,10)
	dec_material("gray1")
	set_material("gray1", .2,.2,.2,1, .7,.7,.7, 10)
	use_material("gray1")
	quad(a,b,c,d)
	quad(e,f,g,h)
	-- Outset
	-- h2 g2
	--
	-- e2 f2
	-- d2 c2
	-- a2 b2
	local a2 = v(-20,0,15)
	local b2 = v(20,0,15)
	local c2 = v(20,40,15)
	local d2 = v(-20,40,15)
	local e2 = v(-20,48,15)
	local f2 = v(20,48,15)
	local g2 = v(20,200,15)
	local h2 = v(-20,200,15)
	dec_material("gray2")
	set_material("gray2", .9,.9,.9,1)
	use_material("gray2")
	-- inset bits
	quad(b,b2,c2,c)
	quad(d,c,c2,d2)
	quad(a,d,d2,a2)
	quad(f,e,e2,f2)
	quad(g,f,f2,g2)
	quad(h,g,g2,h2)
	quad(e,h,h2,e2)
	-- front face
	-- fd fc
	-- fa fb
	local fa = v(-30,0,15)
	local fb = v(30,0,15)
	local fc = v(30,210,15)
	local fd = v(-30,210,15)
	quad(d2,c2,f2,e2)
	quad(fc,fd,h2,g2)
	
	-- tri fan on front face
	xref_tri(g2,f2,c2)
	xref_tri(g2,c2,b2)
	xref_tri(g2,b2,fb)
	xref_tri(g2,fb,fc)

	top = v(0,260,-40)
	-- rear
	xref_quad(fc,fb,v(0,0,-40),v(0,210,-40))
	xref_tri(fc,v(0,210,-40),top)
	-- spire
	spire = v(0,300,-40)
	xref_tri(v(1.5,257.5,-37.25),
		top,spire)
	tri(v(-1.5,257.5,-37.25),v(1.5,257.5,-37.25),
		spire)
	-- front roof triangle section
	--      top
	--      tc
	--
	--   ta   tb
	-- fd       fc
	local ta = v(-21, 215, 9.5)
	local tb = v(21, 215, 9.5)
	local tc = v(0, 250, -29)
	quad(fd,fc,tb,ta)
	xref_quad(fc,top,tc,tb)
	use_material("gray1")
	tri(ta,tb,tc)
end

function clockhand_static()
	dec_material("mat")
	set_material("mat",0,0,1,1, 0,0,0, 10)
	use_material("mat")
	zbias(3, v(0,0,0), v(0,0,1))
	tri(v(-0.06, -0.06, 0), v(0.06, -0.06, 0), v(0, 1, 0))
end

rott = 0
function clock_static()
	dec_material("face")
	dec_material("numbers")
	set_material("face", 1,1,1,1)
	set_material("numbers",.5,.5,0,1)
	use_material("face")
	zbias(1, v(0,0,0), v(0,0,1))
	circle(24, v(0,0,0), v(0,0,1), v(0,1,0), 1.0)
	zbias(2, v(0,0,0), v(0,0,1))

	use_material("numbers")
	text("12", v(0,0.85,0), v(0,0,1), v(1,0,0), 0.15)
	text("1", v(0.425,0.74,0), v(0,0,1), v(1,0,0), 0.15)
	text("2", v(0.74,0.43,0), v(0,0,1), v(1,0,0), 0.15)
	text("3", v(0.85,0,0), v(0,0,1), v(1,0,0), 0.15)
	text("4", v(0.74,-.425,0), v(0,0,1), v(1,0,0), 0.15)
	text("5", v(0.425,-.74,0), v(0,0,1), v(1,0,0), 0.15)
	text("6", v(0,-.85,0), v(0,0,1), v(1,0,0), 0.15)
	text("7", v(-.425,-.736,0), v(0,0,1), v(1,0,0), 0.15)
	text("8", v(-.74,-.425,0), v(0,0,1), v(1,0,0), 0.15)
	text("9", v(-.85,0,0), v(0,0,1), v(1,0,0), 0.15)
	text("10", v(-.74,.425,0), v(0,0,1), v(1,0,0), 0.15)
	text("11", v(-.425,.736,0), v(0,0,1), v(1,0,0), 0.15)

end
function clock_dynamic()
	local t = os.time()
	local handPos = (t%60)/60

	call_model("clockhand", v(0,0,0),
			v(math.cos(-handPos),-math.sin(-handPos),0),
			v(-math.sin(-handPos), -math.cos(-handPos),0), 0.65)
	local handPos = (t%(60*60))/(60*60)
	call_model("clockhand", v(0,0,0),
			v(math.cos(-handPos),-math.sin(-handPos),0),
			v(-math.sin(-handPos), -math.cos(-handPos),0), 0.5)
end
function church_static()
	dec_material("body")
	set_material("body", .5, .5, .3,1)
	use_material("body")
	extrusion(v(0,0,-18), v(0,0,12), v(0,1,0), 1.0,
		v(-7,0,0), v(7,0,0), v(7,10,0), v(-7,10,0))
	extrusion(v(0,0,-18), v(0,0,-4), v(0,1,0), 1.0,
		v(-7,10,0), v(7,10,0), v(7,25,0), v(-7,25,0))
	local roof1 = v(0,20,12)
	local roof2 = v(0,20,-4)
	local spire = v(0,45,-11)
	tri(v(-7,10,12), v(7,10,12), roof1)
	
	dec_material("spire")
	set_material("spire", .8, .3, .0,1)
	use_material("spire")
	xref_quad(roof2, roof1, v(7,10,12), v(7,10,-4))
	xref_tri(spire, v(7,25,-4), v(7,25,-18))
	tri(spire, v(-7,25,-4), v(7,25,-4))
	tri(spire, v(7,25,-18), v(-7,25,-18))

	local clockpos1 = v(-7,18,-11)
	local clockpos2 = v(7,18,-11)
	
	call_model("clock", clockpos1, v(0,0,1), v(0,1,0), 2.5)
	call_model("clock", clockpos2, v(0,0,-1), v(0,1,0), 2.5)
end
function mybuilding_static()
	dec_material("red")
	set_material("red", 1,0,0,1)
	use_material("red")
	xref_tube(64, v(0,0,0), v(0,100,0), v(1,0,0), 20, 40)
end

function towerOfShit_static()
	dec_material("mat1")
	set_material("mat1", 1,1,1,1)
	use_material("mat1")
	for i = 0,20 do
		local _xoff = 10*noise(v(0,i*10,0))
		local _zoff = 10*noise(v(0,0,i*10))
		local _start = v(30+_xoff,i*10,_zoff)
		local _end = v(30+_xoff,10+i*10,_zoff)
		xref_cylinder(16, _start, _end, v(1,0,0), 10.0+math.abs(10*noise(0,0.1*i,0)))
	end
end

function boringHighRise_dynamic()
	use_material("mat1")
	tri(v(-20,0,0),v(20,0,0),v(-20,20,0))
end
function boringHighRise_static()
	dec_material("mat1")
	dec_material("windows")
	set_material("mat1", 0.5,0.5,0.5,1)
	use_material("mat1")
	extrusion(v(0,0,20), v(0,0,-20), v(0,1,0), 1.0,
			v(-20,0,0), v(20,0,0), v(20,200,0), v(-20,200,0))
	set_material("windows", 0,0,0,1, 1,1,1,50, .5,.5,0)
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
	

function test_static()
	dec_material("red")
	dec_material("shinyred")
	set_material("red", 1,0,0,1)
	set_material("shinyred", 1,0,0,0.5, 1,1,1,50)
	use_material("red")
	zbias(1, v(0,5,0), v(0,0,1))
	text("ABCDEfghi Hello 1234", v(0,5,0), v(0,0,1), v(1,1,0):norm(), 1.0)
	zbias(0)
	call_model("blob", v(0,0,-2), v(1,0,0), v(0,1,0),1.0)
	xref_cylinder(16, v(5,0,0), v(10,0,0), v(0,1,0), 1.0)
	use_material("shinyred")
	xref_circle(9, v(4,5,0), v(0,0,1), v(1,0,0), 1.0)
	tri(v(12,3,0),v(13,3,0), v(12,4,0))
	xref_tri(v(13,3,0),v(14,3,0), v(13,4,0))
	xref_quad(v(6,6,0), v(7,6,0), v(7,7,0),v(6,7,0))
---[[	
	xref_bezier_3x3(32,
			v(0,0,0), v(1,-1,0), v(2,0,0),
			v(-1,1,0), v(1,1,8), v(3,1,0),
			v(0,2,0), v(1,3,0), v(2,2,0))
			--]]
--[[	
	xref_bezier_4x4(32,
			v(0,0,0), v(1,0,0), v(2,0,0), v(3,0,0),
			v(0,1,0), v(1,1,5), v(2,1,0), v(3,1,0),
			v(0,2,0), v(1,2,0), v(2,2,0), v(3,2,0),
			v(0,4,0), v(1,4,0), v(1,4,0), v(1,3,0))
			--]]
	extrusion(v(0,0,0), v(0,0,-5), v(0,1,0), 1.0,
		v(1,0,0), v(0.5, 0.8, 0), v(0,1,0), v(-0.5,0.8,0), v(-1,0,0),
		v(0,-0.5,0))
end
poo = 0
function test_dynamic()
	poo = poo + 0.005
	set_material("red", math.sin(poo)*math.sin(poo), 0.5, 0.5, 1)
	use_material("red")
	xref_cylinder(16, v(-8,0,0), v(-8,5,0), v(1,0,0), math.abs(math.sin(poo)))
	circle(9, v(5*math.sin(poo),5*math.cos(poo),0), v(0,0,1), v(1,0,0), 1.0)
end

function blob_static()
	dec_material("blue")
	set_material("blue", 0,0,1,1)
	use_material("blue")
	cylinder(16, v(-5,0,0), v(-5,5,0), v(1,0,0), 1.0)
	text("blob_static()", v(-5,-2,0), v(0,0,1), v(1,0,0), 0.5)
end

function hi(x)
	x = x + 1
	return {
		static=function(val)
			print(tostring(x) .. " sod you " .. tostring(val))
		end,
		
		dynamic=function(val)
			print("you called b("..tostring(val))
		end
	}
end

x = hi(1)
x.static("my friend")
x.dynamic(123)
-- comment

register_models("blob","test","mybuilding", "towerOfShit",
"boringHighRise","clockhand","clock","church", "skyscraper1", "building1",
"building2")
