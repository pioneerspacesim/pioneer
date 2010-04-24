
#include "lua_includes.h"
#include "oolua_userdata.h"
#include "oolua_storage.h"
#include "oolua_push_pull.h"
#include <stdexcept>
#include "oolua_char_arrays.h"

namespace OOLUA
{

	namespace INTERNAL
	{
		//pushes the weak top and returns its index
		int push_weak_table(lua_State* l)
		{
			lua_getfield(l, LUA_REGISTRYINDEX, weak_lookup_name);
			return lua_gettop(l);
		}

		//if found it is left on the top of the stack and returns true
		//else the stack is same as on entrance to the function and false returned
		bool is_there_an_entry_for_this_void_pointer(lua_State* l,void* ptr)
		{
			int wt = push_weak_table(l);
			bool result = is_there_an_entry_for_this_void_pointer(l,ptr,wt);
			lua_remove(l,wt);
			return result;
		}
		bool is_there_an_entry_for_this_void_pointer(lua_State* l,void* ptr,int tableIndex)
		{
			lua_pushlightuserdata(l,ptr);//weakTable ... ptr
			lua_rawget(l,tableIndex);//weakTable .... (full user data or nil)
			if(! lua_isnil(l,-1) )
			{
				return true;//leave ud on top
			}
			lua_pop(l,1);//pop nil
			return false;
		}
		//returns the ud if found and cleans the stack else a NULL pointer
		Lua_ud* find_ud_dont_care_about_type_and_clean_stack(lua_State* /*const*/ l,void* ptr)
		{
			Lua_ud* ud(0);
			if( is_there_an_entry_for_this_void_pointer(l,ptr) )
			{
				ud = static_cast<Lua_ud *>( lua_touserdata(l, -1) );
				lua_pop(l,1);//pop ud
				return ud;
			}
			return ud;
		}


		//on entering user data and weaktable are on the stack
		void add_ptr_if_required(lua_State* const l, void* ptr,int udIndex,int weakIndex)
		{
			lua_pushlightuserdata(l,ptr);//ptr
			lua_rawget(l,weakIndex);//(null or ptr)
			if( lua_isnil(l,-1) == 0 )
			{
				lua_pop(l, 1);//pop the ud
				return;
			}
			lua_pop(l,1);//pop the null

			lua_pushlightuserdata(l,ptr);//key
			lua_pushvalue(l,udIndex);//key ud
			lua_rawset(l,weakIndex);//table[key]=value
		}
		void set_owner( lua_State* l,void* ptr, Owner own)
		{
			if(own == No_change){assert(0);return;}//should never get called but...
			Lua_ud* ud = find_ud_dont_care_about_type_and_clean_stack(l,ptr);
			if(!ud)throw std::runtime_error("(set owner)The pointer was not in the __weak_lookup table");
			ud->gc = ( own == Cpp ? false : true);
		}

		bool ud_at_index_is_const(lua_State* l, int index)
		{
			return INTERNAL::id_is_const( static_cast<Lua_ud *>( lua_touserdata(l, index) ) ); 
		}
	}
}


