
#	include "lua_table.h"
#	include "oolua_exception.h"
#include <cassert>

namespace OOLUA
{
	Lua_table::Lua_table()
		:m_lua(0)
	{}
	Lua_table::Lua_table(lua_State*  const lua,std::string const& name)
		:m_lua(lua),m_table_ref(lua)
	{
		set_table(name);
	}
	Lua_table::Lua_table(Lua_table const& rhs)
		:m_lua(rhs.m_lua),m_table_ref(rhs.m_table_ref)
	{}
	void Lua_table::bind_script(lua_State*  const lua)
	{
		m_lua = lua;
	}
	void Lua_table::set_table(std::string const& name)
	{ 
		if(name.empty())
		{
			Lua_table_ref t;
			m_table_ref.swap(t);
			return;
		}
		if(!m_lua)return;
		lua_getfield(m_lua, LUA_GLOBALSINDEX, name.c_str() );
		if(lua_type(m_lua, -1) != LUA_TTABLE)
		{
			lua_pop(m_lua,1);
			lua_getfield(m_lua, LUA_REGISTRYINDEX, name.c_str() );
			if(lua_type(m_lua, -1) != LUA_TTABLE)
			{
				lua_pop(m_lua,1);
				Lua_table_ref t;
				m_table_ref.swap(t);
				return;
			}
		}
		set_ref( m_lua, luaL_ref(m_lua, LUA_REGISTRYINDEX) );
	}
	bool Lua_table::valid()const
	{ 
		int const init_stack_top = initail_stack_size();
		bool result = get_table();
		restore_stack(init_stack_top);
		return result;
	}

	void Lua_table::set_ref(lua_State* const lua,int const& ref)
	{
		m_lua = lua;
		m_table_ref.set_ref(lua,ref);
	}

	bool Lua_table::get_table()const
	{
		bool result(false);
		if( !m_table_ref.valid() )return result;
		lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_table_ref.ref() );
		return  lua_type(m_lua, -1) == LUA_TTABLE;
	}
	bool Lua_table::push_on_stack(lua_State* l)const
	{
		return m_table_ref.push(l);
	}
	void Lua_table::restore_stack(int const & init_stack_size)const
	{
		//ok now we need to clean up the stack if there are left overs
		if(!m_lua)return;
		int end_stack_size( lua_gettop(m_lua) );
		if(init_stack_size != end_stack_size)
		{
			lua_pop(m_lua,end_stack_size - init_stack_size);
		}
	}
	int Lua_table::initail_stack_size()const
	{
		int size = (!m_lua)? 0 : lua_gettop(m_lua);
		assert(size >= 0);
		return size;
		//return (!m_lua)? 0 : lua_gettop(m_lua);
	}
	void Lua_table::traverse(Lua_table::traverse_do_function do_)
	{
		if(! get_table() )return;
		int t(1);
		lua_pushnil(m_lua);  /* first key */
		while (lua_next(m_lua, t) != 0) 
		{
			(*do_)(m_lua);
			lua_pop(m_lua, 1);
		}
		lua_pop(m_lua, 1);
	}
	void Lua_table::swap(Lua_table & rhs)
	{
		m_table_ref.swap(rhs.m_table_ref);
		lua_State* ts(m_lua);
		m_lua = rhs.m_lua;
		rhs.m_lua = ts;
	}
	
	void new_table(lua_State* l,OOLUA::Lua_table& t)
	{
		//lua_newtable(l);
		//OOLUA::Lua_table temp;
		//temp.swap(t);
		//OOLUA::pull2cpp(l,t);
		new_table(l).swap(t);

	}
	OOLUA::Lua_table new_table(lua_State* l)
	{
		lua_newtable(l);
		OOLUA::Lua_table t;
		OOLUA::pull2cpp(l,t);
		return t;
		
	}

}

