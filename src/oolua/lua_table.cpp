
#	include "lua_table.h"
#	include "oolua_exception.h"
#	include "fwd_push_pull.h"

namespace OOLUA
{
	Lua_table::Lua_table()
	{}
	Lua_table::Lua_table(Lua_table_ref const& ref)
		:m_table_ref(ref)
	{}
	Lua_table::Lua_table(lua_State*  const lua,std::string const& name)
		:m_table_ref(lua)
	{
		set_table(name);
	}
	Lua_table::Lua_table(Lua_table const& rhs)
		:m_table_ref(rhs.m_table_ref)
	{}
	void Lua_table::bind_script(lua_State*  const lua)
	{
		if(m_table_ref.valid() )
		{
			Lua_table_ref tempRef(lua);
			m_table_ref.swap(tempRef);
	}
		else m_table_ref.m_lua = lua;

	}
	void Lua_table::set_table(std::string const& name)
	{ 
		if(name.empty())
		{
			Lua_table_ref t;
			m_table_ref.swap(t);
			return;
		}
		if(!m_table_ref.m_lua)return;
		//REMOVE
		//lua_getfield(m_table_ref.m_lua, LUA_GLOBALSINDEX, name.c_str() );
		lua_getglobal(m_table_ref.m_lua,name.c_str() );
		if(lua_type(m_table_ref.m_lua, -1) != LUA_TTABLE)
		{
			lua_pop(m_table_ref.m_lua,1);
			lua_getfield(m_table_ref.m_lua, LUA_REGISTRYINDEX, name.c_str() );
			if(lua_type(m_table_ref.m_lua, -1) != LUA_TTABLE)
			{
				lua_pop(m_table_ref.m_lua,1);
				Lua_table_ref t;
				m_table_ref.swap(t);
				return;
			}
		}
		set_ref( m_table_ref.m_lua, luaL_ref(m_table_ref.m_lua, LUA_REGISTRYINDEX) );
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
		m_table_ref.set_ref(lua,ref);
	}

	
	bool Lua_table::get_table()const
	{
		bool result(false);
		if( !m_table_ref.valid() )return result;
		lua_rawgeti(m_table_ref.m_lua, LUA_REGISTRYINDEX, m_table_ref.ref() );
		return  lua_type(m_table_ref.m_lua, -1) == LUA_TTABLE;
	}

	bool Lua_table::push_on_stack(lua_State* l)const
	{
		return m_table_ref.push(l);
	}
	bool Lua_table::pull_from_stack(lua_State* l)
	{
		return m_table_ref.pull(l);
	}
	void Lua_table::lua_pull_from_stack(lua_State* l)
	{
		m_table_ref.lua_pull(l);
	}
	void Lua_table::restore_stack(int const & init_stack_size)const
	{
		//ok now we need to clean up the stack if there are left overs
		if(!m_table_ref.m_lua)return;
		int end_stack_size( lua_gettop(m_table_ref.m_lua) );
		if(init_stack_size != end_stack_size)
		{
			lua_pop(m_table_ref.m_lua,end_stack_size - init_stack_size);
		}
	}
	int Lua_table::initail_stack_size()const
	{
		return (!m_table_ref.m_lua)? 0 : lua_gettop(m_table_ref.m_lua);
	}
	void Lua_table::traverse(Lua_table::traverse_do_function do_)
	{
		if(! get_table() )return;
		int t(1);
		lua_pushnil(m_table_ref.m_lua);  /* first key */
		while (lua_next(m_table_ref.m_lua, t) != 0) 
		{
			(*do_)(m_table_ref.m_lua);
			lua_pop(m_table_ref.m_lua, 1);
		}
		lua_pop(m_table_ref.m_lua, 1);
	}
	void Lua_table::swap(Lua_table & rhs)
	{
		m_table_ref.swap(rhs.m_table_ref);
	}

	void new_table(lua_State* l,Lua_table& t)
	{
		new_table(l).swap(t);
	}
	
	Lua_table new_table(lua_State* l)
	{
		lua_newtable(l);
		Lua_table t;
		t.pull_from_stack(l);
		return t;
	}

}

