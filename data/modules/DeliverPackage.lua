
local deliver_flavours = {
	{
		adtext = "GOING TO %0? Money paid for delivery of a small package.",
		introtext = "Hi, I'm %0. I'll pay you %1 if you will deliver a small package to %2.",
		whysomuchdoshtext = "When a friend visited me she left behind some clothes and antique paper books. I'd like to have them returned to her.",
		successmsg = "Thank you for the delivery. You have been paid in full.",
		failuremsg = "Jesus wept, you took forever over that delivery. I'm not willing to pay you.",
		danger = 0,
		time = 300,
		money = 50,
	}, {
		adtext = "WANTED. Delivery of a package to %0.",
		introtext = "Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		whysomuchdoshtext = "It is nothing special.",
		successmsg = "The package has been received and you have been paid in full.",
		failuremsg = "I'm frustrated by the late delivery of my package, and I refuse to pay you.",
		danger = 0,
		time = 100,
		money = 100,
	}, {
		adtext = "URGENT. Fast ship needed to deliver a package to %0.",
		introtext = "Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		whysomuchdoshtext = "It is a research proposal and must be delivered by the deadline or we may not get funding.",
		successmsg = "You have been paid in full for the delivery. Thank you.",
		failuremsg = "I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
		danger = 0,
		time = 75,
		money = 110,
	}, {
		adtext = "DELIVERY. Documents to the %0 system. %1 to an experienced pilot.",
		introtext = "Hello. I'm %0. I'm willing to pay %1 for a ship to carry a package to %2.",
		whysomuchdoshtext = "Some extremely sensitive documents have fallen into my hands, and I have reason to believe that the leak has been traced to me.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		danger = 150,
		time = 75,
		money = 250,
	}
}

-- XXX need
-- 1 Format_money
-- 1 ImportantMessage()
--   SBodyPath currentSystem->GetRandomStarportNearButNotIn
--   NaturalSpaceStationName(SBodyPath*)
--   NaturalSystemName(SBodyPath)
--   currentSystem->GetPathOf(Pi::player->GetDockedWith()->GetSBody(), &path)

for i = 0,10 do
	local sys = SystemId:new(i,0,0)
	print(sys:GetName())
	print(sys:GetShortDescription())
end

Module:new {
	__name = 'DeliverPackage',

	Init = function(self)
		self.x = SBodyPath:new()
		self.s = SystemId:new(1,0,0)
		print(self.s:GetShortDescription())
	end,
	Unserialize = function(self,data)
		Module.Unserialize(self, data)
		print("yay! "..self.s:GetName())
	end,
		
}
