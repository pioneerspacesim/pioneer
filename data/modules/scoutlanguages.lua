  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Scout', {
  	adtext = "RECON NEEDED: {system} System, distance {dist} ly. {cash}",
		introtext = "Hello, my name is {name}. My organization has some concerns about illegal nuclear and chemical weapons testing being performed on <{systembody}> in {system} ({sectorx}, {sectory}, {sectorz}). Distance of {dist} ly. We are willing to pay {cash} for an atmosphere analyzation.",
		introtext2 = "If you are seeing this, then the scan has been completed.\nHead to:",
		whysomuchtext = "It is the standard market rate. I can pay less, if you'd like.",
		successmsg = "Thank you for the info. Your reward is being transferred.",
		failuremsg = "You are far too late, another pilot has completed the scan.",
		urgency = 0,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "EXPLORER NEEDED: {system},  {dist} ly. {cash}",
		introtext = "Hello. I'm {name}. I will pay {cash} for a mapping of < {systembody} > in {system} ({sectorx}, {sectory}, {sectorz}) at a distance of {dist} ly.",
		introtext2 = "You are seeing this because < {systembody} >  was successfully mapped. Thank you, this will prove very useful to my ... friends.\n To complete the mission, you must go to:",
		whysomuchtext = "I thought it was the standard rate; I could offer less?",
		successmsg = "I received the data, thanks! I'm transferring the money now.",
		failuremsg = "You are very late. The info is useless to me now.",
		urgency = 0.1,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "JUNK RECON:  {system}, distance {dist} ly. {cash} Help us keep the system clean!",
		introtext = "Greetings. I am {name}.  My company specializes in the clean up of neglected environments, and is offering {cash} for a thorough junk and atmosphere scan of < {systembody} > in {system} ({sectorx}, {sectory}, {sectorz}) at a distance of {dist} ly.",
		introtext2 = "The scan is complete. Bring the information to:",
		whysomuchtext = "We will spare no expensive in the pursuit of clean planets.",
		successmsg = "Well done pilot!  This will aid our efforts greatly. You have earned this pay.",
		failuremsg = "Your failure has been noted. We will not be contacting you again.",
		urgency = 0.1,
		risk = 0.1,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "URGENT RECON:  {system}, distance {dist} ly. {cash} Need a discrete and fast pilot for a scan.",
		introtext = "My name is {name}, I am a journalist and I'm writing a story about the system {system} ({sectorx}, {sectory}, {sectorz}) at a distance of {dist} ly. I need a scan  < {systembody} > quickly and I will pay {cash} to someone who can get this information in time.",
		introtext2 = "If you are seeing this, the scan is complete. Deliver data to:",
		whysomuchtext = "I'ma renowned journalist, not someone who writes for cheap leaflets. I need this information quickly and am willing to pay for a professional.",
		successmsg = "Thank you. The agreed upon amount should already be in your account",
		failuremsg = "You are far too late and someone has already performed the scan.",
		urgency = 0.8,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "RECON: in {system}, distance {dist} ly. {cash} for a bold and experienced pilot.",
		introtext = "Hello. My name is {name}. My job is to find people who do not want to be found. I believe a person of interest is located on  < {systembody} >  in {system} ({sectorx}, {sectory}, {sectorz}) at a distance of {dist} ly. I will pay {cash} ",
		introtext2 = "The scan has been completed. Head immediatly to :",
		whysomuchtext = "I believe that a very wanted man is hiding out in the area. He may be dangerous.",
		successmsg = "Your punctuality and discretion is appreciated. You will now receive the promised reward.",
		failuremsg = "Useless! I will not hire you again! .",
		urgency = 0.4,
		risk = 0.9,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "LOCAL MAPPING: {system}'s government needs your help collecting census data",
		introtext = "Nice to meet you, I'm {name}. Due to a shortage of pilots, we are looking to pay an independant ship owner {cash} for a current bio-scan of < {systembody} > . No rush, just need to keep our files updated.",
		introtext2 = "The mapping has been successful\n To complete it should be directed to:",
		whysomuchtext = "Our government rewards those who assist the common good well.",
		successmsg = "Thank you for helping. Your reward is transferred already.",
		failuremsg = "You are late. The data has already been provided.",
		urgency = 0.1,
		risk = 0,
		localscout = 1,
})

Translate:AddFlavour('English','Scout', {
		adtext = "URGENT LOCAL RECON: The {police} of {system} needs your help to protect its inhabitants.",
		introtext = "Hello! I'm Captain {name}, {police} of {system}. We urgently need a scan of < {systembody} >. We will pay {cash} .",
		introtext2 = "If you are seeing this, the first part of mission has been successful. To complete, go to:",
		whysomuchtext = "We must ensure the security of our region. You will recieve information on a need to know basis.",
		successmsg = "We appreciate the quick work, pilot. Your money has been transferred.",
		failuremsg = "You should be fined for this! We advise leaving our system.",
		urgency = 0.8,
		risk = 0.4,
		localscout = 1,
})

Translate:Add({ English = {
  ["additional information"] = "If you accept this mission, you will program your mapper with what I'm looking for. This program will make decisions for me and report back to your computer. If you should come here, or bring the data to another system, depends on what you find. It may occur that change the delivery destination to another more convenient system. You must be alert and assured of enough fuel and time.\nThis does not apply to local missions.",

  ["message risk 00-02-1"] = "Don't be ridiculous, you are in no danger.",
	["message risk 00-02-2"] = "There is no reason to be worried; this a routine scan.  However, pirates are everywhere these days",
  ["message risk 03-05-1"] = "I suspect that there is some strange activity. Probably nothing serious, but you better be prepared.",
  ["message risk 06-08-1"] = "There has been reports of pirate activity in the area; including a frigate that disappeared in the area.",
  ["message risk 09-10-1"] = "Several ships have been lost in the area, including my previous contractor. I really need to know what is happening so if you don't have combat experience, don't bother.",
  ["message risk 09-10-2"] = "You will definitely enounter resistance.  There is likely a pirate base in the area and pirates do not take kindly to those who scan their base.",

  ["I need the information by "] = "For security reasons, the program will destroy all traces of this mission at the expiration of the time limit. It is very important that you give me the results before:       ",
  ["Scout"] = "Scout",
  ["Excellent. I await your report."] = "Excellent. I await your report.",
  ["Why so much money"] = "Why so much money, why are you paying?",
  ["Have additional information"] = "I need additional information.",
  ["When do you need the data"] = "When do you need the data?",
  ["What is the risk"] = "What is the risk?",
  ["Repeat the original request"] = "Can you repeat the original request?",
  ["Ok"] = "Ok, we have a deal.",
  ["ly"] = "ly",
  
  -- Texts for the missions screen
  scoutmissiondetail = [[
	Objective:
	System:
	Deadline:
                                      
	Distance:]],

  scoutmissiondetail2 = [[
  Station:
  System:
  Deadline:
                                      
  Distance:]],

  ["Distance reached"] = "DISTANCE REACHED. SCANNING. Maintain orbit ",
	["minutes"] = " minutes.",
  ["MAPPING interrupted."] = "SCANNING INTERRUPTED. You are out of scanner range. Maintain correct orbit.",
  ["COMPLETE MAPPING"] = "SCANNING COMPLETE. Data saved. Check delivery destination.",
  ["computer"] = "Computer",
	["You have not installed RADAR MAPPER"] = "You must have a Radar Mapper to perform this task.\nPlease go to the Shipyard and you can purchase one from the equipment section.",
	["You will be paid on my behalf in new destination."] = "You will be paid on my behalf in new destination",
	["Unauthorized data here is REMOVED"] = "Unauthorized scan data has been detected and removed.",
	["You have been fined "] = "You have been fined ",
 HostileMessages = {
	"Check this out, we've got the rat in sight! Do not let him leave!",
	"Your ship and your life belongs to us!",
	"Are you willing to die for that data? You should leave now.",
	"You will never leave here with that scan!",
  },
}, })
