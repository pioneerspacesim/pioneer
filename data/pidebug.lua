-- Pioneer Lua debugging system

-- Note: this file is required to start Pioneer.
-- It must return a table, which must contain a function called error_handler.

return {
	error_handler =
		function (message)
			local trace = debug.traceback(message, 2)

			fl = io.open(Engine.userdir .. '/lua-core-dump', 'w')
			fl:write(trace)
			fl:close()
			return (trace)
		end
}
