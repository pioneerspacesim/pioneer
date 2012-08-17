EventQueue = {
	Create = function (name)
		local events = {}
		local callbacks = {}

		EventQueue["on"..name] = {
			Connect = function (self, cb)
				callbacks[cb] = cb
			end,

			Disconnect = function (self, cb)
				callbacks[cb] = nil
			end,

			DebugTimer = function (self, enabled)
				-- implement this
			end,

			Queue = function (...)
				table.insert(events, {...})
			end,

			Signal = function (...)
				local e = {...}
				for cb,_ in pairs(callbacks) do
					cb(table.unpack(e))
				end
			end,

			Emit = function ()
				while #events > 0 do
					local e = table.remove(events, 1)
					for cb,_ in pairs(callbacks) do
						cb(table.unpack(e))
					end
				end
			end,

			ClearEvents = function ()
				events = {}
			end,
		}
	end
}

EventQueue.Create("GameStart");
EventQueue.Create("GameEnd");
EventQueue.Create("EnterSystem");
EventQueue.Create("LeaveSystem");
EventQueue.Create("FrameChanged");
EventQueue.Create("ShipDestroyed");
EventQueue.Create("ShipHit");
EventQueue.Create("ShipCollided");
EventQueue.Create("ShipDocked");
EventQueue.Create("ShipUndocked");
EventQueue.Create("ShipLanded");
EventQueue.Create("ShipTakeOff");
EventQueue.Create("ShipAlertChanged");
EventQueue.Create("Jettison");
EventQueue.Create("CargoUnload");
EventQueue.Create("AICompleted");
EventQueue.Create("CreateBB");
EventQueue.Create("UpdateBB");
EventQueue.Create("SongFinished");
EventQueue.Create("ShipFlavourChanged");
EventQueue.Create("ShipEquipmentChanged");
EventQueue.Create("ShipFuelChanged");
