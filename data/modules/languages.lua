-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

  -- adtext - text shown in the bulletin board list
  -- introtext - shown when the advert is selected (and "Could you repeat request?")
  -- expeditiontext - response to "Why so much money?"
  -- howmany - response to "How many of you are there?"
  -- danger - repsponse to "Will I be in any danger?"
  -- successmsg - message sent on successful taxi service
  -- failuremsg - message sent on failed taxi service after docking
  -- wherearewe - message sent on failed taxi service after entering system
  -- single - 1 if the taxi service is for single person
  -- urgency - how urgent the transport is
  -- risk - how risky the mission is. 0 is none. 1 is certain death

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "We are visiting a friend.",
  danger = "No.",
  scientist =  
  corporation =  "Sirius"
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "We work for {corp} corporation and they are paying.",
  danger = "No.",
  scientist = 
  corporation = "ACME",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "It's normal business trip.",
  danger = "No.",
    scientist = 
  corporation = "Taranis",
  successmsg = "Thanks for bringing us here. We've now paid in full. Good Luck!",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP REQUIRED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "An old rival is trying to kill me.",
  danger = "I think there is an assassin on my trail, and they may come after you.",
    scientist = 
  corporation = "Aquarian Shipbuilding",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "I'm traveling salesman.",
  danger = "No.",
  scientist = 
  corporation = "Rockforth",
  successmsg = "Thanks for carrying me here. I've paid in full. Good Luck!",
  failuremsg = "Don't even ask for payment! I'm reporting you to the authorities.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "Didn't you know - I'm a well known dream star.",
  danger = "You might get some interest from the press. Just ignore them.",
  scientist = 
  corporation = "Amaliel",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "What have you done! My tour is all spoilt and I lost half of my fans.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "I'm freelance journalist.",
  danger = "No.",
  scientist = 
  corporation = "Marett Space",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Safe passage to {system} system. Will pay {cash}.",
  introtext = "Hi, my name is {name} and I need safe passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "The Mafia want me dead.",
  danger = "The Mafia don't take kindly to people helping their enemies.",
  scientist = 
  corporation =  "Vega Line",
  successmsg = "Thanks for carrying me safely here. I've paid in full. Good Luck!",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I'm visiting a sick relative.",
  danger = "No.",
  scientist = 
  corporation =  "Digital",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "The Police want me to help them with their enquiries.",
  danger = "The Police may try to stop you.",
  scientist = 
  corporation =  "Bulk Ships",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Useless! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I want fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Paying {cash}.",
  expeditiontext = "I would rather someone didn't find me.",
  danger = "I think someone is following me.",
  scientist = 
  corporation = "Arment Aerodynamics"
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "You are so inexperienced pilot. Not going to pay for this.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "FAST SHIP: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I'm a factory inspector doing my rounds.",
  danger = "Sometimes people don't want to be inspected.",
  scientist = 
  corporation = "Cool Cola",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "I'm going to lost my job because of your incompetence. So I need that money more than you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP REQUIRED: Passage to {system} system.",
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I owe someone some money, and they're after me.",
  danger = "Someone is chasing me.",
  scientist = 
  corporation = "Weyland
  successmsg = "Thank you for the ride. You have been paid in full.",
  failuremsg = "I do not have enough money. Sorry.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ English = {
  ["Expedition"] = "Expedition",
  ["Why so much money?"] = "Why so much money?",
  ["What kind of expedition are we talking about?"] = "What kind of expedition are we talking about?",
  ["You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t."] = "You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t.",
  ["When will we arrive back?"] = "When will we arrive back?",
  ["Will I be in any danger?"] = "Will I be in any danger?",
  ["I must be there before "] = "I must be there before ",
  ["We want to be there before "] = "We want to be there before ",
  ["You do not have enough cabin space on your ship."] = "You do not have enough cabin space on your ship.",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["Ok, agreed."] = "Ok, agreed.",
  ["Hey!?! You are going to pay for this!!!"] = "Hey!?! You are going to pay for this!!!",
  ["ly"] = "ly",

  -- Texts for the missions screen
  taximissiondetail = [[
  From:
  To:
  Group details:
  Danger:
  Deadline:

  Distance:]],

 PIRATE_TAUNTS = {
  "You're going to regret dealing with {client}",
	"You have {client} on board? That was a bad idea.",
	"Today isn't your lucky day! Prepare to die.",
	"You're not going to dock today!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 
	 
	 "Aquarian Shipbuilding",
	 
	 
	 
	
	
	
	 
  },
}, })

