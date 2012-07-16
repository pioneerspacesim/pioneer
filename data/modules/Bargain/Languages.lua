
-- bargain 1 is a 10% more than the stock market, 10 is a 100% more than the stock market.

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Bargain', {
	whyneededtext = "I have a ban on trade on the local stock market, i need a {CommodityNeeded} immediately.",
	bargain = 5,
})

Translate:AddFlavour('English','Bargain', {
	whyneededtext = "{CommodityNeeded} on the local stock market are poor quality.",
	bargain = 2,
})

Translate:AddFlavour('English','Bargain', {
	whyneededtext = "It's not your business.",
	bargain = 10,
})

Translate:AddFlavour('English','Bargain', {    -- Do not trust anyone ;)
	whyneededtext = "It's not your business.",
	bargain = -2,
})

Translate:Add({ English = {
	["Hi, I'm {Name}. I'll need {CommodityNeeded}."] = "Hi, I'm {Name}. I'll need {CommodityNeeded}.",
	["{CommodityNeeded} needed."] = "{CommodityNeeded} needed.",
	["Why not buy on the stock market?"] = "Why not buy on the stock market?",
	["Sell {CommodityNeeded}"] = "Sell {CommodityNeeded}",
	["Announcement outdated."] = "Announcement outdated.",
	["I need {CommodityNeeded}, but like i see you cannot help me."] = "I need {CommodityNeeded}, but like i see you cannot help me.",
	},
})

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','Bargain', {
	whyneededtext = "Mam zakaz handlu na tutejszej giełdzie, a {CommodityNeeded} potrzebuję natychmiast.",
	bargain = 5,
})

Translate:AddFlavour('Polski','Bargain', {
	whyneededtext = "{CommodityNeeded} na tutejszej giełdzie są marnej jakości.",
	bargain = 2,
})

Translate:AddFlavour('Polski','Bargain', {
	whyneededtext = "To nie twoja sprawa.",
	bargain = 10,
})

Translate:AddFlavour('Polski','Bargain', {
	whyneededtext = "To nie twoja sprawa.",
	bargain = -2,
})

Translate:Add({ Polski = {
	["Hi, I'm {Name}. I'll need {CommodityNeeded}."] = "Cześć, jestem {Name}. Potrzebuję {CommodityNeeded}.",
	["{CommodityNeeded} needed."] = "{CommodityNeeded} potrzebne.",
	["Why not buy on the stock market?"] = "Dlaczego nie kupisz na giełdzie?",
	["Sell {CommodityNeeded}"] = "Sprzedaj {CommodityNeeded}",
	["Announcement outdated."] = "Ogłoszenie nieaktualne.",
	["I need {CommodityNeeded}, but like i see you cannot help me."] = "Potrzebuję {CommodityNeeded}, ale jak widzę nie możesz mi pomóc.",
	},
})
