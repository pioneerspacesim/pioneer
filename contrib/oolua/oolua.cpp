///////////////////////////////////////////////////////////////////////////////
///  @file oolua.cpp
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details.
///////////////////////////////////////////////////////////////////////////////
#	include "oolua.h"
#   include "oolua_storage.h"
#   include "oolua_check_result.h"
#	include "oolua_error.h"
#	include "oolua_char_arrays.h"
namespace
{
	void add_weaklookup_table(lua_State* l)
	{
		lua_newtable(l);//tb
		int weakTable = lua_gettop(l);
		luaL_newmetatable(l, "weak_mt");
		int weak_mt = lua_gettop(l);

		lua_pushstring(l,"__mode");//tb mt key
		lua_pushstring(l,"v");//tb mt key value
		lua_settable(l,weak_mt);//tb mt
		//weak_mt["__mode"]="v"

		lua_setmetatable(l, weakTable);//tb
		//weakTable["__mt"]=weak_mt

		OOLUA::INTERNAL::Weak_table::setWeakTable(l,-2);
		//registry[weak_lookup_name]=weakTable
		
		lua_pop(l,1);//empty
	}
	void add_ownership_globals(lua_State* l)
	{
		//REMOVE
		/*
		push_char_carray(l,OOLUA::INTERNAL::cpp_owns_str);//string
		lua_pushinteger(l,OOLUA::Cpp);//string int
		lua_settable(l, LUA_GLOBALSINDEX);//globals[string]=int
		*/
		lua_pushinteger(l,OOLUA::Cpp);//string int
		lua_setglobal(l,OOLUA::INTERNAL::cpp_owns_str);//globals[string]=int
		

		/*
		push_char_carray(l,OOLUA::INTERNAL::lua_owns_str);//string
		lua_pushinteger(l,OOLUA::Lua);//string int
		lua_settable(l, LUA_GLOBALSINDEX);//globals[string]=int
		*/
		
		lua_pushinteger(l,OOLUA::Lua);//string int
		lua_setglobal(l,OOLUA::INTERNAL::lua_owns_str);//globals[string]=int
	}
}

namespace OOLUA
{
	void setup_user_lua_state(lua_State* l)
	{
		add_weaklookup_table(l);
		add_ownership_globals(l);
	}

	Script::Script(): call(),m_lua(0)
	{
		m_lua = luaL_newstate();
		luaL_openlibs(m_lua);
		
		//temp fix to correct openlibs leaving entries on the stack
		//in Lua 5.2 work 3
		int top = lua_gettop(m_lua);
		if(top != 0)lua_pop(m_lua,top);
		//
		
		
		call.bind_script(m_lua);//bind the lua state to the function caller
		setup_user_lua_state(m_lua);
	}

	Script::~Script()
	{
		close_down();
	}

	void Script::gc()
	{
		lua_gc(m_lua,LUA_GCCOLLECT,0);
	}

	void Script::close_down()
	{
		if(m_lua)
		{
			lua_gc(m_lua,LUA_GCCOLLECT,0);
			lua_close(m_lua); m_lua = 0;
		}
	}

	bool Script::load_chunk(std::string const& chunk)
	{
		int res = luaL_loadbuffer(m_lua,chunk.c_str(),chunk.size(),"userChunk");
		return INTERNAL::load_buffer_check_result(m_lua,res);
	}

	bool Script::run_chunk(std::string const& chunk)
	{
		if(! load_chunk(chunk ) ) return false;
		int result = lua_pcall(m_lua,0,LUA_MULTRET,0);
		return INTERNAL::protected_call_check_result(m_lua,result);
	}

	bool Script::run_file(std::string const & filename/*,bool multi_return*/)
	{
		bool status = load_file(filename);
		if( !status )return false;
		int result = lua_pcall(m_lua,0,LUA_MULTRET,0);
		return INTERNAL::protected_call_check_result(m_lua,result);
	}

	bool Script::load_file(std::string const & filename)
	{
		int result = luaL_loadfile(m_lua, filename.c_str() );
		return INTERNAL::load_buffer_check_result(m_lua,result);;
	}

}

