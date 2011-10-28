-- Pioneer Lua debugging system

-- Note: this file is required to start Pioneer.
-- It must return a table, which must contain a function called error_handler.

local function functionEntryString(func)
	local info
	if type(func) == 'table' then
		info = func
	elseif type(func) == 'function' or type(func) == 'number' then
		info = debug.getinfo(func)
	else
		return '!!! NOT A FUNCTION !!!'
	end
	local floc = ''
	local fname = ''
	if info.currentline and info.currentline ~= -1 then
		floc = string.format(' @ %s:%d', info.short_src, info.currentline)
	end
	if info.name and info.name ~= '' then
		local fnamewhat = ''
		if info.namewhat ~= '' then
			fnamewhat = string.format(' (from %s)', info.namewhat)
		end
		fname = string.format(' %q%s', info.name, fnamewhat)
	end
	return string.format('%s function%s%s', info.what, fname, floc)
end

--[==[
test_table = { [1] = 'hello', [2] = 'world', [3] = { 'one', 'two', 'three' } }
test_table.loop = test_table[3]
test_table.func = function (answer)
	print(string.format('And the winner is... %d!', answer))
end
--]==]

local function parseableValue(value, onlyReadable, pretty)
	local seenTables = {}
	if type(onlyReadable) ~= 'boolean' then
		onlyReadable = true
	end
	if type(pretty) ~= 'boolean' then
		pretty = false
	end
	local function buildValue(value, onlyReadable, pretty, indent)
		local ty = type(value)
		local indentStr = pretty and string.rep('  ', indent) or ''

		if ty == 'string' then
			return string.format('%q', value)
		elseif ty == 'table' then
			local entries = {}
			-- break cycles in tables...
			if seenTables[value] then
				return '"<repeated table...>"'
			else
				seenTables[value] = true
				for k,v in pairs(value) do
					entries[#entries + 1] =
						string.format('%s[%s] = %s',
							indentStr,
							buildValue(k, onlyReadable, false, 0),
							buildValue(v, onlyReadable, pretty, indent+1))
				end
				if pretty then
					return '{\n' .. table.concat(entries, ',\n') .. '\n' .. string.rep('  ', indent-1) .. '}'
				else
					return '{ ' .. table.concat(entries, ', ') .. ' }'
				end
			end
		elseif ty == 'function' then
			local finfo = debug.getinfo(value, 'u')
			if onlyReadable or finfo.nups ~= 0 then
				return string.format('%q', functionEntryString(value))
			else
				return string.format('loadstring(%q)', string.dump(value))
			end
		elseif ty == 'thread' then
			-- might be able to give more info than this with debug.getinfo()
			-- to find out the current stack state of the thread
			return string.format('%q', '<' .. tostring(value) .. '>')
		elseif ty == 'userdata' then
			-- this is slightly dangerous: tostring() can fall into a
			-- metamethod which might not be valid during debug
			return string.format('%q', tostring(value))
		else
			-- tostring() does the right thing for number, boolean and nil
			return tostring(value)
		end
	end
	return buildValue(value, onlyReadable, pretty, 1)
end

local function valueToString(value)
	local ty = type(value)
	if ty == 'string' then
		return string.format('%q', value)
	elseif ty == 'userdata' then
		return '<' .. tostring(value) .. '>'
	elseif ty == 'function' then
		return '<' .. functionEntryString(value) .. '>'
	elseif ty == 'table' then
		return parseableValue(value, true)
	else
		return tostring(value)
	end
end

local function dumpEnv(env)
	return parseableValue(env, true, true)
end

local function dumpStack(top_level)
	if type(top_level) ~= 'number' or top_level < 1 then
		top_level = 1
	end

	local lines = {}
	local level = top_level + 1 -- add 1 to skip debug.getinfo() itself
	local finfo = debug.getinfo(level)
	while finfo ~= nil do
		-- subtract 2 from level to pretend getinfo and dumpStack don't exist
		lines[#lines + 1] = string.format('--[%d]-- %s', level - 2, functionEntryString(finfo))
		if finfo.currentline ~= -1 then
			lines[#lines + 1] = string.format('currently on line %d', finfo.currentline)
		end

		do -- print locals
			local varidx = 1
			local varname, varval = debug.getlocal(level, varidx)
			if varname ~= nil then
				lines[#lines + 1] = 'Locals:'
			end
			while varname ~= nil do
				lines[#lines + 1] = string.format('  %s = %s', varname, parseableValue(varval, true))
				varidx = varidx + 1
				varname, varval = debug.getlocal(level, varidx)
			end
		end

		-- print up-values
		if finfo.nups > 0 then
			local func = finfo.func

			lines[#lines + 1] = 'Up-values:'

			local varidx = 1
			local varname, varval = debug.getupvalue(func, varidx)
			while varname ~= nil do
				lines[#lines + 1] = string.format('  %s = %s', varname, parseableValue(varval, true))
				varidx = varidx + 1
				varname, varval = debug.getupvalue(func, varidx)
			end
		end

		local fenv = debug.getfenv(finfo.func)
		if fenv ~= _G then
			lines[#lines + 1] = 'Environment (' .. tostring(fenv) .. '):'
			lines[#lines + 1] = dumpEnv(fenv)
		end

		level = level + 1
		finfo = debug.getinfo(level)
	end
	return table.concat(lines, '\n')
end

return {
	error_handler =
		function (message)
			local trace = debug.traceback(message, 2) -- level 0 is debug.traceback; level 1 is error_handler

			fl = io.open(Engine.userdir .. '/lua-core-dump', 'w')
			fl:write(message)
			fl:write('\n\n')
			fl:write('### STACK TRACE\n')
			fl:write(dumpStack(2)) -- dump from level 2 (0 is dumpStack, 1 is error_handler)
			fl:write('\n\n')
			fl:write('### GLOBALS\n')
			fl:write(dumpEnv(_G))
			fl:write('\n\n')
			fl:close()
			return (trace)
		end
}
