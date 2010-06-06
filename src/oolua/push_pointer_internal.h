
#ifndef PUSH_POINTER_INTERAL_H_
#	define PUSH_POINTER_INTERAL_H_

#include "lua_includes.h"
#include "param_traits.h"
#include "oolua_userdata.h"
#include "oolua_storage.h"


namespace OOLUA
{

	namespace INTERNAL
	{
		template<typename T>void push_pointer(lua_State * /*const*/ l, T* const ptr,Owner owner);
		template<typename T>void push_const_pointer(lua_State * /*const*/ l, T const* const ptr,Owner owner);


		void if_check_enabled_check_type_is_registered(lua_State* l, char const* name);
		void set_owner_if_change(Owner owner, Lua_ud* ud);
		
		template<typename T>
		inline void push_pointer(lua_State * /*const*/ l, T* const ptr,Owner owner)
		{			
			if_check_enabled_check_type_is_registered(l,Proxy_class<T>::class_name);
			Lua_ud* ud( find_ud(l,ptr,false) );
			if(! ud ) ud = add_ptr(l,ptr,false);

			set_owner_if_change(owner,ud);
		}
		template<typename T>
		inline void push_const_pointer(lua_State * /*const*/ l, T const* const ptr,Owner owner)
		{
			if_check_enabled_check_type_is_registered(l,Proxy_class<T>::class_name);
			Lua_ud* ud( find_ud(l,(T* const)ptr,true ) );
			if(! ud ) ud = add_ptr(l,(T* const)ptr,true);

			set_owner_if_change(owner,ud);
		}
	}
}

#endif

