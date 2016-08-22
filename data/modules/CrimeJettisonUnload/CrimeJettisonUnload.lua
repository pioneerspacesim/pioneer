local t = Translate:GetTranslator()

local messages = {
    POLICE          = 'Police',
    CRIME_JETTISON  = 'You are not allowed to jettison {cargo} here! Your crime has been recorded.',
    CRIME_UNLOAD    = 'You are not allowed to unload {cargo} here! Your crime has been recorded.',
}

local onJettison = function(player, cargo)
    if player:IsPlayer() then
        if cargo.type == "RUBBISH" then
            UI.ImportantMessage( string.interp( t( messages['CRIME_JETTISON'] ), { cargo = t( cargo.type ) } ), t( messages['POLICE'] ) )
            --- player:AddCrime("TRADING_ILLEGAL_GOODS", 500)
        elseif cargo.type == "RADIOACTIVES" then
            UI.ImportantMessage( string.interp( t( messages['CRIME_JETTISON'] ), { cargo = t( cargo.type ) } ), t( messages['POLICE'] ) )
            --- player:AddCrime("TRADING_ILLEGAL_GOODS", 500)
        end
    end
end

local onCargoUnload = function(player, cargo)
    if player:IsPlayer() then
        if cargo.type == "RUBBISH" then
            UI.ImportantMessage( string.interp( t( messages['CRIME_UNLOAD'] ), { cargo = t( cargo.type ) } ), t( messages['POLICE'] ) )
            --- player:AddCrime("TRADING_ILLEGAL_GOODS", 1000)
        elseif cargo.type == "RADIOACTIVES" then
            UI.ImportantMessage( string.interp( t( messages['CRIME_UNLOAD'] ), { cargo = t( cargo.type ) } ), t( messages['POLICE'] ) )
            --- player:AddCrime("TRADING_ILLEGAL_GOODS", 1000)
        end
    end
end

EventQueue.onJettison:Connect(onJettison)
--- Only available in alpha 18 and above
--- EventQueue.onCargoUnload:Connect(onCargoUnload)