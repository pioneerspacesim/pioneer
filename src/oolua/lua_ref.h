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

#include "oolua_config.h"
#include "oolua_error.h"


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
		bool lua_push(lua_State* const lua)const;
		bool lua_pull(lua_State* const lua);
	private:
		friend class  Lua_table;
		bool pull_if_valid(lua_State* l);
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
		if( valid() )
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
#if OOLUA_RUNTIME_CHECKS_ENABLED == 1
		else if( lua != m_lua )
		{
#	if OOLUA_USE_EXCEPTIONS ==1
			lua_pushstring(lua, "Can not push a valid Lua reference from one lua_State to a different state");
			throw OOLUA::Runtime_error(lua);
#	endif
#	if OOLUA_STORE_LAST_ERROR ==1
			lua_pushfstring(lua,"Can not push a valid Lua reference from one lua_State to a different state"); 
			OOLUA::INTERNAL::set_error_from_top_of_stack_and_pop_the_error(lua);
#	endif
#	if OOLUA_DEBUG_CHECKS == 1
		//	assert(0 && "Can not push a valid Lua reference from one lua_State to a different state");
#	endif

#	if OOLUA_USE_EXCEPTIONS == 0
			return false;
#	endif
		}
#endif
		lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_ref );
		return  lua_type(m_lua, -1) == ID;
	}
	
	template<int ID>
	bool Lua_ref<ID>::lua_push(lua_State* const lua)const
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
	bool Lua_ref<ID>::pull_if_valid(lua_State* const l)
	{
		if( lua_type(l, -1) == ID )
		{
			set_ref( l, luaL_ref(l, LUA_REGISTRYINDEX) );
			return true;
		}
		else if( lua_type(l,-1) ==LUA_TNIL && lua_gettop(l) >=1)
		{
			release();
			return true;
		}
		return false;
	}
	template<int ID>
	bool Lua_ref<ID>::pull(lua_State* const lua) 
	{
		if( !pull_if_valid(lua) )
		{
			////NOT IN C++ handle the error
#if OOLUA_DEBUG_CHECKS ==1
			assert( 0 &&  "pulling incorrect type from stack");
#endif
			return false;
		}
		return true;
	}
	
	
	template<int ID>
	bool Lua_ref<ID>::lua_pull(lua_State* const lua) 
	{
		if( !pull_if_valid(lua) )
		{
			luaL_error(lua,
					   "pulling incorrect type from stack. This is a ref to id %d, stack contains %s"
					   ,ID
					   ,lua_typename(lua,lua_type(lua, -1))
						);
			return false;
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
