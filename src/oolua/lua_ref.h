///////////////////////////////////////////////////////////////////////////////
///  @file lua_ref.h
///  A wrapper for a lua registry reference in a struct 
///  so that the type is different to an int. Typedefs two types Lua_table_ref & \n
///  Lua_func_ref.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef LUA_REF_H_
#	define LUA_REF_H_

#include "lua_includes.h"
#include <cassert>

#ifdef __GNUC__
#	define OOLUA_DEFAULT __attribute__ ((visibility("default")))
#else
#	define OOLUA_DEFAULT
#endif	

namespace OOLUA
{
	class Lua_table;

	///////////////////////////////////////////////////////////////////////////////
	///  @class Lua_func_ref
	///  A wrapper for a lua registry reference in a struct 
	///  so that the type is different to an int.
	///
	///  @remarks 
	///  An object wrapper for a value in LUA_REGISTRYINDEX
	///  @author Liam Devine and David O'Reilly @date 01/03/2007
	///////////////////////////////////////////////////////////////////////////////
	template<int ID>
	struct Lua_ref
	{
		Lua_ref(lua_State* const lua,int const& ref);
		explicit Lua_ref(lua_State* const lua);
		Lua_ref();
		Lua_ref& operator =(Lua_ref const& /*rhs*/);//unimplemented
		Lua_ref(Lua_ref const& rhs) OOLUA_DEFAULT;
		~Lua_ref()OOLUA_DEFAULT;
		bool valid()const;
		int const& ref()const;
		void set_ref(lua_State* const lua,int const& ref)OOLUA_DEFAULT;
		void swap(Lua_ref & rhs);
		bool push(lua_State* const lua)const;
		bool pull(lua_State* const lua) OOLUA_DEFAULT ;
	private:
		friend class  Lua_table;
		void release();
		lua_State* m_lua;
		int m_ref;
	};
	

	template<int ID>
	Lua_ref<ID>::Lua_ref(lua_State* const lua,int const& ref)
		:m_lua(lua),m_ref(ref)
	{}
	template<int ID>
	Lua_ref<ID>::Lua_ref(lua_State* const lua)
		:m_lua(lua),m_ref(LUA_NOREF)
	{}
	template<int ID>
	Lua_ref<ID>::Lua_ref()
		:m_lua(0),m_ref(LUA_NOREF)
	{}
	
	template<int ID>
	Lua_ref<ID>::Lua_ref(Lua_ref<ID> const& rhs)
		:m_lua(0),m_ref(LUA_NOREF)
	{
		if (rhs.valid()) 
		{
			m_lua = rhs.m_lua;
			lua_rawgeti(m_lua, LUA_REGISTRYINDEX, rhs.m_ref );
			m_ref = luaL_ref(m_lua, LUA_REGISTRYINDEX);
		}
	}
	
	template<int ID>
	Lua_ref<ID>::~Lua_ref()
	{
		release();
	}
	template<int ID>
	bool Lua_ref<ID>::valid()const
	{
		return m_lua && m_ref != LUA_REFNIL && m_ref != LUA_NOREF;
	}
	template<int ID>
	int const& Lua_ref<ID>::ref()const
	{
		return m_ref;
	}
	template<int ID>
	inline void Lua_ref<ID>::set_ref(lua_State* const lua,int const& ref)
	{
		release();
		m_ref = ref;
		m_lua = lua;
	}
	template<int ID>
	void Lua_ref<ID>::release()
	{
		if(/*m_lua*/ valid() )
		{
			luaL_unref(m_lua,LUA_REGISTRYINDEX,m_ref);
		}
		m_ref = LUA_NOREF;
	}
	template<int ID>
	void Lua_ref<ID>::swap(Lua_ref & rhs)
	{
		lua_State* tl (rhs.m_lua);
		int tr (rhs.m_ref);
		rhs.m_lua = m_lua;
		rhs.m_ref = m_ref;
		m_lua = tl;
		m_ref = tr;
	}

	template<int ID>
	bool Lua_ref<ID>::push(lua_State* const lua)const
	{
		if (!valid() ) {
			lua_pushnil(lua);
			return true;
		}
		else if( lua != m_lua )
		{
			luaL_error(lua,"The reference is not valid for this Lua State");
			return false;
		}
		lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_ref );
		return  lua_type(m_lua, -1) == ID;
	}

	template<int ID>
	bool Lua_ref<ID>::pull(lua_State* const lua) 

	{
		if( lua_type(lua, -1) == ID )
		{
			set_ref( lua, luaL_ref(lua, LUA_REGISTRYINDEX) );
		}
		else if( lua_type(lua,-1) ==LUA_TNIL && lua_gettop(lua) >=1)
		{
			release();
		}
		else
		{
			assert( 0 &&  "pulling incorrect type from stack");
		}
		return true;
	}
	///\typedef Lua_table_ref
	///Wrapper for a lua table
	typedef Lua_ref<LUA_TTABLE> Lua_table_ref;//0
	///\typedef Lua_func_ref 
	///Wrapper for a lua function
	typedef Lua_ref<LUA_TFUNCTION> Lua_func_ref;//1


}
#endif //LUA_REF_H_
