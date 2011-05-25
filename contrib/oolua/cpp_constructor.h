#ifndef OOLUA_CPP_CONSTRUCTOR_H_
#	define OOLUA_CPP_CONSTRUCTOR_H_

///////////////////////////////////////////////////////////////////////////////
///  @file cpp_constructor.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#	include "lua_includes.h"
#	include "oolua_storage.h"
#	include "param_traits.h"
#	include "oolua_userdata.h"
#	include "member_func_helper.h"
#	include "oolua_parameter_helper.h"
namespace OOLUA
{
namespace INTERNAL
{
template<typename Type,int HasNoDefaultTypedef>
struct Constructor
{
	static int construct(lua_State * l)
	{
		Type* obj = new Type;
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
		return 1;
	}
};
template<typename Type>
struct Constructor<Type,1>
{
	static int construct(lua_State * l)
	{
		luaL_error(l,"%s %s %s","No parameters passed to the constructor of the type"
			,OOLUA::Proxy_class<Type>::class_name
			,"which does not have a default constructor.");
		return 0;
	}
};
template<typename Class,typename Param1WithTraits >
struct Constructor1
{
	static int construct(lua_State* l) 
	{
		if(param_is_of_type<Param1WithTraits >(l,1))
		{
			valid_construct(l);
			return 1;
		}
		return 0;
	}
	static void valid_construct(lua_State* l)
	{
		typename Param1WithTraits::pull_type p1;
		OOLUA::Member_func_helper<Param1WithTraits,Param1WithTraits::owner>::pull2cpp(l,p1);
		OOLUA::Converter<typename Param1WithTraits::pull_type,typename Param1WithTraits::type> p1_(p1);
		Class* obj = new Class( p1_);
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
	}
};
template<typename Class,typename Param1WithTraits, typename Param2WithTraits >
struct Constructor2
{
	static int construct(lua_State* l) 
	{
		if(param_is_of_type<Param1WithTraits >(l,1)
			&& param_is_of_type<Param2WithTraits >(l,2))
		{
			valid_construct(l);
			return 1;
		}
		return 0;
	}
	static void valid_construct(lua_State* l)
	{
		typename Param2WithTraits::pull_type p2;
		OOLUA::Member_func_helper<Param2WithTraits,Param2WithTraits::owner>::pull2cpp(l,p2);
		OOLUA::Converter<typename Param2WithTraits::pull_type,typename Param2WithTraits::type> p2_(p2);
		typename Param1WithTraits::pull_type p1;
		OOLUA::Member_func_helper<Param1WithTraits,Param1WithTraits::owner>::pull2cpp(l,p1);
		OOLUA::Converter<typename Param1WithTraits::pull_type,typename Param1WithTraits::type> p1_(p1);
		Class* obj = new Class( p1_,p2_);
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
	}
};
template<typename Class,typename Param1WithTraits, typename Param2WithTraits, typename Param3WithTraits >
struct Constructor3
{
	static int construct(lua_State* l) 
	{
		if(param_is_of_type<Param1WithTraits >(l,1)
			&& param_is_of_type<Param2WithTraits >(l,2)
			&& param_is_of_type<Param3WithTraits >(l,3))
		{
			valid_construct(l);
			return 1;
		}
		return 0;
	}
	static void valid_construct(lua_State* l)
	{
		typename Param3WithTraits::pull_type p3;
		OOLUA::Member_func_helper<Param3WithTraits,Param3WithTraits::owner>::pull2cpp(l,p3);
		OOLUA::Converter<typename Param3WithTraits::pull_type,typename Param3WithTraits::type> p3_(p3);
		typename Param2WithTraits::pull_type p2;
		OOLUA::Member_func_helper<Param2WithTraits,Param2WithTraits::owner>::pull2cpp(l,p2);
		OOLUA::Converter<typename Param2WithTraits::pull_type,typename Param2WithTraits::type> p2_(p2);
		typename Param1WithTraits::pull_type p1;
		OOLUA::Member_func_helper<Param1WithTraits,Param1WithTraits::owner>::pull2cpp(l,p1);
		OOLUA::Converter<typename Param1WithTraits::pull_type,typename Param1WithTraits::type> p1_(p1);
		Class* obj = new Class( p1_,p2_,p3_);
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
	}
};
template<typename Class,typename Param1WithTraits, typename Param2WithTraits, typename Param3WithTraits, typename Param4WithTraits >
struct Constructor4
{
	static int construct(lua_State* l) 
	{
		if(param_is_of_type<Param1WithTraits >(l,1)
			&& param_is_of_type<Param2WithTraits >(l,2)
			&& param_is_of_type<Param3WithTraits >(l,3)
			&& param_is_of_type<Param4WithTraits >(l,4))
		{
			valid_construct(l);
			return 1;
		}
		return 0;
	}
	static void valid_construct(lua_State* l)
	{
		typename Param4WithTraits::pull_type p4;
		OOLUA::Member_func_helper<Param4WithTraits,Param4WithTraits::owner>::pull2cpp(l,p4);
		OOLUA::Converter<typename Param4WithTraits::pull_type,typename Param4WithTraits::type> p4_(p4);
		typename Param3WithTraits::pull_type p3;
		OOLUA::Member_func_helper<Param3WithTraits,Param3WithTraits::owner>::pull2cpp(l,p3);
		OOLUA::Converter<typename Param3WithTraits::pull_type,typename Param3WithTraits::type> p3_(p3);
		typename Param2WithTraits::pull_type p2;
		OOLUA::Member_func_helper<Param2WithTraits,Param2WithTraits::owner>::pull2cpp(l,p2);
		OOLUA::Converter<typename Param2WithTraits::pull_type,typename Param2WithTraits::type> p2_(p2);
		typename Param1WithTraits::pull_type p1;
		OOLUA::Member_func_helper<Param1WithTraits,Param1WithTraits::owner>::pull2cpp(l,p1);
		OOLUA::Converter<typename Param1WithTraits::pull_type,typename Param1WithTraits::type> p1_(p1);
		Class* obj = new Class( p1_,p2_,p3_,p4_);
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
	}
};
template<typename Class,typename Param1WithTraits, typename Param2WithTraits, typename Param3WithTraits, typename Param4WithTraits, typename Param5WithTraits >
struct Constructor5
{
	static int construct(lua_State* l) 
	{
		if(param_is_of_type<Param1WithTraits >(l,1)
			&& param_is_of_type<Param2WithTraits >(l,2)
			&& param_is_of_type<Param3WithTraits >(l,3)
			&& param_is_of_type<Param4WithTraits >(l,4)
			&& param_is_of_type<Param5WithTraits >(l,5))
		{
			valid_construct(l);
			return 1;
		}
		return 0;
	}
	static void valid_construct(lua_State* l)
	{
		typename Param5WithTraits::pull_type p5;
		OOLUA::Member_func_helper<Param5WithTraits,Param5WithTraits::owner>::pull2cpp(l,p5);
		OOLUA::Converter<typename Param5WithTraits::pull_type,typename Param5WithTraits::type> p5_(p5);
		typename Param4WithTraits::pull_type p4;
		OOLUA::Member_func_helper<Param4WithTraits,Param4WithTraits::owner>::pull2cpp(l,p4);
		OOLUA::Converter<typename Param4WithTraits::pull_type,typename Param4WithTraits::type> p4_(p4);
		typename Param3WithTraits::pull_type p3;
		OOLUA::Member_func_helper<Param3WithTraits,Param3WithTraits::owner>::pull2cpp(l,p3);
		OOLUA::Converter<typename Param3WithTraits::pull_type,typename Param3WithTraits::type> p3_(p3);
		typename Param2WithTraits::pull_type p2;
		OOLUA::Member_func_helper<Param2WithTraits,Param2WithTraits::owner>::pull2cpp(l,p2);
		OOLUA::Converter<typename Param2WithTraits::pull_type,typename Param2WithTraits::type> p2_(p2);
		typename Param1WithTraits::pull_type p1;
		OOLUA::Member_func_helper<Param1WithTraits,Param1WithTraits::owner>::pull2cpp(l,p1);
		OOLUA::Converter<typename Param1WithTraits::pull_type,typename Param1WithTraits::type> p1_(p1);
		Class* obj = new Class( p1_,p2_,p3_,p4_,p5_);
		OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr(l,obj,false);
		ud->gc = true;
	}
};

}
}
#define OOLUA_CONSTRUCTORS_BEGIN \
static int oolua_factory_function(lua_State* l) \
{ \
	lua_remove(l, 1);/*remove class type*/ \
	int stack_count = lua_gettop(l); \
	if(stack_count == 0 ) \
	{ \
		return OOLUA::INTERNAL::Constructor<class_,has_typedef<this_type, No_default_constructor>::Result>::construct(l); \
	}

#define OOLUA_CONSTRUCTOR_1(param1Type) \
	if(stack_count == 1) \
	{ \
		if(OOLUA::INTERNAL::Constructor1<class_,param_type<param1Type > >::construct(l) ) return 1; \
	}

#define OOLUA_CONSTRUCTOR_2(param1Type,param2Type) \
	if(stack_count == 2) \
	{ \
		if(OOLUA::INTERNAL::Constructor2<class_,param_type<param1Type >,param_type<param2Type > >::construct(l) ) return 1; \
	}

#define OOLUA_CONSTRUCTOR_3(param1Type,param2Type,param3Type) \
	if(stack_count == 3) \
	{ \
		if(OOLUA::INTERNAL::Constructor3<class_,param_type<param1Type >,param_type<param2Type >,param_type<param3Type > >::construct(l) ) return 1; \
	}

#define OOLUA_CONSTRUCTOR_4(param1Type,param2Type,param3Type,param4Type) \
	if(stack_count == 4) \
	{ \
		if(OOLUA::INTERNAL::Constructor4<class_,param_type<param1Type >,param_type<param2Type >,param_type<param3Type >,param_type<param4Type > >::construct(l) ) return 1; \
	}

#define OOLUA_CONSTRUCTOR_5(param1Type,param2Type,param3Type,param4Type,param5Type) \
	if(stack_count == 5) \
	{ \
		if(OOLUA::INTERNAL::Constructor5<class_,param_type<param1Type >,param_type<param2Type >,param_type<param3Type >,param_type<param4Type >,param_type<param5Type > >::construct(l) ) return 1; \
	}

#define OOLUA_CONSTRUCTORS_END \
	luaL_error(l,"%s %d %s %s","Could not match",stack_count,"parameter constructor for type",class_name); \
	return 0; \
}

#define OOLUA_ONLY_DEFAULT_CONSTRUCTOR \
OOLUA_CONSTRUCTORS_BEGIN \
OOLUA_CONSTRUCTORS_END



#endif 
