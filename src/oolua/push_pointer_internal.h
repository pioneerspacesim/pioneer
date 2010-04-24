
#ifndef PUSH_POINTER_INTERAL_H_
#	define PUSH_POINTER_INTERAL_H_

#include "lua_includes.h"
#include "param_traits.h"
#include "oolua_userdata.h"
#include "oolua_storage.h"

#if defined DEBUG || defined _DEBUG
#	define DEBUG_LUA_CHECKING
#endif

namespace OOLUA
{

	namespace INTERNAL
	{
		template<typename T>void push_pointer(lua_State * /*const*/ l, T* const ptr,Owner owner);
		template<typename T>void push_const_pointer(lua_State * /*const*/ l, T const* const ptr,Owner owner);

#ifdef DEBUG_LUA_CHECKING
		inline void if_check_enabled_check_type_is_registered(lua_State* l, char const* name)
		{
			lua_getfield(l, LUA_REGISTRYINDEX, name);
			if( lua_isnil(l,-1) )
			{
				lua_pop(l,1);
				luaL_error(l,"%s %s %s","the type",name,"is not registered with this Lua State");
			}
			else lua_pop(l,1);
		}
#else
		inline void if_check_enabled_check_type_is_registered(lua_State* /*l*/, char const* /*name*/){}
#endif

		template<typename T>
		inline void push_pointer(lua_State * /*const*/ l, T* const ptr,Owner owner)
		{			
			if_check_enabled_check_type_is_registered(l,Proxy_class<T>::class_name);
			Lua_ud* ud( find_ud(l,ptr,false) );
			if(! ud ) ud = add_ptr(l,ptr,false);

			if(owner != No_change)
			{
				ud->gc =( owner == Lua ? true : false);
			}
		}
		template<typename T>
		inline void push_const_pointer(lua_State * /*const*/ l, T const* const ptr,Owner owner)
		{
			if_check_enabled_check_type_is_registered(l,Proxy_class<T>::class_name);
			Lua_ud* ud( find_ud(l,(T* const)ptr,true ) );
			if(! ud ) ud = add_ptr(l,(T* const)ptr,true);

			if(owner != No_change)
			{
				ud->gc =( owner == Lua ? true : false);
			}
		}
	}
}

#endif

