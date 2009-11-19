function poo()
	x = average(1,2,3,4,5);
end

x = v(1,2,3,5)+v(2,3,4)
x = x-v(3,1,1)
x:print()
y = v(math.sin(10),math.cos(10),0):cross(v(0,1,0))
y:print()

z = 3.1*v(1,2,3)
z:print()

z = v(1,2,3)/10
z = z:norm()
z:print()
a = v(z:len(), z:dot(y), z:len())
a:print()
print(v(1,0,0):dot(v(0.5,1,0):norm()))

m = Model.new{name="myModel", radius=1.2, scale=10}
print (m)
