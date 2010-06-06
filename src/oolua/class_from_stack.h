
#ifndef CLASS_FROM_STACK_H_
#	define CLASS_FROM_STACK_H_

#	include "lua_includes.h"
#	include "fwd_push_pull.h"
#	include "oolua_typedefs.h"
#	include "proxy_class.h"
#	include "lua_table.h"
#	include "oolua_userdata.h"

//#define OOLUA_ABSOLUTE_LUA_INDEX(L, i)
//		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

#include "oolua_error.h"
namespace OOLUA
{


	namespace INTERNAL
	{

		template<typename T>
		T * check_index(lua_State * l, int narg);

		template<typename T>
		T  * check_index_no_const(lua_State * l, int narg);

		bool index_is_userdata(lua_State* l,int index);
		bool get_metatable_and_check_type_is_registered(lua_State* l,int const& index,char const * name);
		bool is_requested_type_a_base(lua_State* l,INTERNAL::Lua_ud* requested_ud,int const& userdata_index);
		
		template<typename T>
		inline T* class_from_stack_top(lua_State * l)
		{
			return check_index<T>(l,lua_gettop(l));
		}

		template<typename T>
		inline T* none_const_class_from_stack_top(lua_State * l)
		{
			return check_index_no_const<T>(l,lua_gettop(l));
		}

		template<typename T>
		inline T* class_from_index(lua_State * l,int index)
		{
#if OOLUA_DEBUG_CHECKS == 1
			assert(index >0);
#endif
			return check_index<T>(l,index);
		}

		template<typename T>
		inline T* none_const_class_from_index(lua_State * l,int index)
		{
#if OOLUA_DEBUG_CHECKS == 1
			assert(index >0);
#endif
			return check_index_no_const<T>(l,index);
		}


		template<typename T>
		T* valid_base_ptr_or_null(lua_State* l,int userdata_index)
		{
			INTERNAL::Lua_ud requested_ud;
			requested_ud.none_const_name = (char*) OOLUA::Proxy_class<T>::class_name;
			requested_ud.name_size = OOLUA::Proxy_class<T>::name_size;
			if(!is_requested_type_a_base(l,&requested_ud,userdata_index))
			{
				//ud ...
				return (T*)0;
			}
			else  
			{
				//ud ... typed_class_ptr 
				T* t = static_cast<T* >(lua_touserdata(l, -1));
				lua_pop(l,1);//ud ...  
				return t;
			}
		}


		template<typename T>
		T* check_index(lua_State * /*const*/ l, int narg)
		{
			if( ! index_is_userdata(l,narg ))
				return 0;
			INTERNAL::Lua_ud * ud = static_cast<INTERNAL::Lua_ud *>( lua_touserdata(l, narg) );

			if(! INTERNAL::ids_equal(ud->none_const_name,ud->name_size
								,(char*)Proxy_class<T>::class_name,Proxy_class<T>::name_size) )
			{
#if OOLUA_RUNTIME_CHECKS_ENABLED == 0
				lua_getmetatable(l,narg);//userdata ... stackmt
#endif
				return valid_base_ptr_or_null<T>(l,narg);
			}
#if OOLUA_RUNTIME_CHECKS_ENABLED == 1
			lua_pop(l,1);
#endif
			return static_cast<T* >(ud->void_class_ptr);
		}


		template<typename T>
		T* check_index_no_const(lua_State * l, int narg)
		{
			if( ! index_is_userdata(l,narg )) 
				return 0;
			INTERNAL::Lua_ud * ud = static_cast<INTERNAL::Lua_ud *>( lua_touserdata(l, narg) );

			if( INTERNAL::id_is_const(ud) )
			{
				luaL_error (l, "%s \"%s\" %s", "Tried to pull a none constant"
					,OOLUA::Proxy_class<T>::class_name
					,"pointer from a const pointer"
					);
				return (T*)0;
			}
			if( ! INTERNAL::ids_equal(ud->none_const_name,ud->name_size
									,(char*)Proxy_class<T>::class_name,Proxy_class<T>::name_size) )
			{
#if OOLUA_RUNTIME_CHECKS_ENABLED == 0
				lua_getmetatable(l,narg);//userdata ... stackmt
#endif
				return valid_base_ptr_or_null<T>(l,narg);
			}
#if OOLUA_RUNTIME_CHECKS_ENABLED == 1
			lua_pop(l,1);
#endif
			return static_cast<T* >(ud->void_class_ptr);
		}
		
		template<typename T>
		T* no_stack_checks_none_const_class_from_index(lua_State* l, int narg)
		{
			INTERNAL::Lua_ud * ud = static_cast<INTERNAL::Lua_ud *>( lua_touserdata(l, narg) );
			if( INTERNAL::id_is_const(ud) )
			{
				//NOTE: proxy caller via Lua code called
				luaL_error (l, "%s \"%s\" %s", "Tried to pull a none constant"
							,OOLUA::Proxy_class<T>::class_name
							,"pointer from a const pointer"
							);
				return (T*)0;
			}
			if( ! INTERNAL::ids_equal(ud->none_const_name,ud->name_size
									  ,(char*)Proxy_class<T>::class_name,Proxy_class<T>::name_size) )
			{
				lua_getmetatable(l,narg);//userdata ... stackmt
				return valid_base_ptr_or_null<T>(l,narg);
			}
			return static_cast<T* >(ud->void_class_ptr);
		}
		
		template<typename T>
		T* no_stack_checks_class_from_index(lua_State * /*const*/ l, int narg)
		{
			INTERNAL::Lua_ud * ud = static_cast<INTERNAL::Lua_ud *>( lua_touserdata(l, narg) );
			if(! INTERNAL::ids_equal(ud->none_const_name,ud->name_size
									 ,(char*)Proxy_class<T>::class_name,Proxy_class<T>::name_size) )
			{
				lua_getmetatable(l,narg);//userdata ... stackmt
				return valid_base_ptr_or_null<T>(l,narg);
			}
			
			return static_cast<T* >(ud->void_class_ptr);
		}
	}

}
#endif

