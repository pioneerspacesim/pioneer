#ifndef OOLUA_BASE_CHECKER_H_
#	define OOLUA_BASE_CHECKER_H_

#include "type_list.h"
#include "lua_includes.h"
#include "oolua_userdata.h"
#include "proxy_class.h"

namespace OOLUA
{
    template<typename T>
	int stack_top_type_is_base(lua_State* const l
								,INTERNAL::Lua_ud* requested_ud
								,int const& userdata_index);

	namespace INTERNAL
	{

		template<typename ProxyStackType,typename BaseType,int DoWork>
		struct CastToRequestedProxyType;

        template<typename ProxyStackType,typename Bases, int BaseIndex,typename BaseType>
		struct Is_a_base;


		//cast the class pointer to the correct type and put it onto
		//of the stack
		template<typename ProxyStackType,typename BaseType,int DoWork = 1>
		struct CastToRequestedProxyType
		{
			static void cast(lua_State* const l,int const& userdata_index)
			{
				//get the userdata
				OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud*>( lua_touserdata(l, userdata_index) );
				//cast the class void ptr from the stack to the stacktype
				//then to base type to get correct offset
				BaseType* baseptr = static_cast<typename ProxyStackType::class_* > ( ud->void_class_ptr );
				//push class pointer of requested type onto stack
				lua_pushlightuserdata(l,baseptr);
			}
		};
		template<typename ProxyStackType,typename BaseType>
		struct CastToRequestedProxyType<ProxyStackType,BaseType,0>
		{
			static void cast(lua_State* const /*l*/,int const& /*userdata_index*/)
			{}
		};

		template<typename ProxyStackType,typename Bases, int BaseIndex,typename BaseType>
		struct Is_a_base
		{
			void operator()(lua_State * const l,int const& userdata_index,INTERNAL::Lua_ud* requested_ud,bool & result)
			{
				if(result) return;
				//is this a base
				if( INTERNAL::ids_equal(requested_ud->none_const_name,requested_ud->name_size,
					(char*)OOLUA::Proxy_class<BaseType>::class_name,OOLUA::Proxy_class<BaseType>::name_size ) )
				{
					result = true;
					CastToRequestedProxyType<ProxyStackType,BaseType,1>::cast(l,userdata_index);
				}
				if(result) return;
				//check the next in the type list
				Is_a_base<
					ProxyStackType
					,Bases
					,BaseIndex + 1
					,typename TYPELIST::At_default< Bases, BaseIndex + 1, TYPE::Null_type >::Result
				> nextIsBase;
				nextIsBase(l,userdata_index,requested_ud,result);
			}
		};
		template<typename ProxyStackType,typename Bases, int BaseIndex>
		struct Is_a_base<ProxyStackType,Bases,BaseIndex,TYPE::Null_type>
		{
			void operator()(lua_State * const /*l*/,int const& /*userdata_index*/,INTERNAL::Lua_ud* /*requested_ud*/,bool & /*result*/)
			{
				return;
			}
		};

	}


	template<typename T>
	inline int stack_top_type_is_base(lua_State* const l
										,INTERNAL::Lua_ud* requested_ud
										,int const& userdata_index)
	{
		//ud... 
		INTERNAL::Is_a_base<OOLUA::Proxy_class<T>
			,typename OOLUA::Proxy_class<T>::AllBases
			,0
			,typename TYPELIST::At_default< typename OOLUA::Proxy_class<T>::AllBases,0,TYPE::Null_type >::Result
		> checkBases;
		bool result(false);
		checkBases(l,userdata_index,requested_ud,result);
		return !!result;
	}
}
#endif
