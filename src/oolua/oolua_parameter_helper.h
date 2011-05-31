#ifndef OOLUA_PARAMETER_HELPER_H_
#	define OOLUA_PARAMETER_HELPER_H_

#	include "param_traits.h"
#	include "oolua_userdata.h"
#	include "lua_includes.h"
#	include "class_from_stack.h"
#	include "oolua_config.h"

namespace OOLUA
{
	namespace INTERNAL
	{
		
		template<typename TypeWithTraits,int Is_intergral>
		struct index_can_convert_to_type
		{
			static int valid(lua_State* /*l*/,int const& /*index*/){return 0;}//noop
		};
		
		template<typename TypeWithTraits>
		struct index_can_convert_to_type<TypeWithTraits,0>
		{
			static int valid(lua_State* l,int index)
			{
MSC_PUSH_DISABLE_CONDTIONAL_CONSTANT_OOLUA
				if( !TypeWithTraits::is_constant
				   && INTERNAL::id_is_const(static_cast<INTERNAL::Lua_ud *>( lua_touserdata(l, index) ) )  )
				{
					return 0;
				}
				return OOLUA::INTERNAL::class_from_index<typename TypeWithTraits::raw_type>(l,index)!=0;
MSC_POP_COMPILER_WARNING_OOLUA
			}

		};	
		template<typename ParamWithTraits>
		int param_is_of_type(lua_State* l,int const& index)
		{
			int lua_stack_type = lua_type(l,index);
			switch (lua_stack_type) 
			{
				case LUA_TBOOLEAN:
					return lua_type_is_cpp_type<typename ParamWithTraits::raw_type,LUA_TBOOLEAN>::value;
					break;
				case LUA_TNUMBER:
					return lua_type_is_cpp_type<typename ParamWithTraits::raw_type,LUA_TNUMBER>::value;
					break;
				case LUA_TSTRING:
					return lua_type_is_cpp_type<typename ParamWithTraits::raw_type,LUA_TSTRING>::value;
				case LUA_TUSERDATA:
					return index_can_convert_to_type<ParamWithTraits,ParamWithTraits::is_integral>::valid(l,index);
					break;
				case LUA_TFUNCTION:
					return lua_type_is_cpp_type<typename ParamWithTraits::raw_type,LUA_TFUNCTION>::value;
					break;
				case LUA_TTABLE:
					return lua_type_is_cpp_type<typename ParamWithTraits::raw_type,LUA_TTABLE>::value;
					break;
					
				default:
					return 0;
					break;
			}
		}
	}
}

#endif
