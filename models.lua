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
	callmodel("blob", v(0,0,-2), v(1,0,0), v(0,1,0),1.0)
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

x = v(1,2,3.2)+v(2,3,4)
x = x-v(3,1,1)
x:print()
y = v(math.sin(10),math.cos(10),0):cross(v(0,1,0))
y:print()
print (x:x())
print (x:y())
print (x:z())

z = 3.1*v(1,2,3)
z:print()

z = v(1,2,3)/10
z = z:norm()
z:print()
a = v(z:len(), z:dot(y), z:len())
a:print()
print(v(1,0,0):dot(v(0.5,1,0):norm()))

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

register_models("blob","test","mybuilding", "towerOfShit", "boringHighRise")
