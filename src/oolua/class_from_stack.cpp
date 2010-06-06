#include "class_from_stack.h"
#include "oolua_config.h"


namespace OOLUA
{
	namespace INTERNAL
	{
#if OOLUA_RUNTIME_CHECKS_ENABLED == 1
		//return : true if id index is an OOLua created userdate type 
		//NOTE: if result is true then the metatable is left on the stack
		bool index_is_userdata(lua_State* l,int index)
		{
			if( !lua_isuserdata(l,index) )return false;
			if(!lua_getmetatable(l, index)) return false;
			lua_pushlightuserdata(l, l);
			lua_rawget(l,-2);
			bool result = lua_isnil(l,-1) ==1 ? false : true;
			
			//pop metatable
			if(result)lua_pop(l,1);
			//pop metatable and also the lookup result on top of the stack
			else lua_pop(l,2);
			
			return result;	
		}
#else
		bool index_is_userdata(lua_State* l,int index)
		{
			return lua_isuserdata(l,index) ? true : false;
		}
#endif


		typedef int(*function_sig_to_check_base)(lua_State* const l,INTERNAL::Lua_ud*,int const&);

		bool is_requested_type_a_base(lua_State* l,INTERNAL::Lua_ud* requested_ud,int const& userdata_index)
		{
			//ud ... stackmt 
			lua_pushlightuserdata(l,l);//ud ... stackmt lua_State*
			lua_rawget(l,-2);//ud ... stackmt   fun
			function_sig_to_check_base isRequestTypeaBaseOfStackType (reinterpret_cast<function_sig_to_check_base>( lua_tocfunction(l,-1) ) );
			lua_pop(l,2);//ud ... 
			return isRequestTypeaBaseOfStackType(l,requested_ud,userdata_index) ? true : false;
		}
	}
}