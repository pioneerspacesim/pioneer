///////////////////////////////////////////////////////////////////////////////
///  @file cpp_member_func.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef CPP_MEMBER_FUNC_H_
#	define CPP_MEMBER_FUNC_H_

#	include "param_traits.h"
#	include "oolua_paramater_macros.h"
#define OOLUA_CONST_FUNC const
#define OOLUA_NON_CONST_FUNC

//member function macros
#define OOLUA_MEM_FUNC_0(return_value,func)\
	LUA_CLASS_MEMBER_FUNCTION_0(func,return_value,func,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_0_CONST(return_value,func)\
	LUA_CLASS_MEMBER_FUNCTION_0(func,return_value,func,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_0_RENAME(name,return_value,func)\
	LUA_CLASS_MEMBER_FUNCTION_0(name,return_value,func,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_0_CONST_RENAME(name,return_value,func)\
	LUA_CLASS_MEMBER_FUNCTION_0(name,return_value,func,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_1(return_value,func,P1)\
	LUA_CLASS_MEMBER_FUNCTION_1(func,return_value,func,P1,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_1_CONST(return_value,func,P1)\
	LUA_CLASS_MEMBER_FUNCTION_1(func,return_value,func,P1,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_1_RENAME(name,return_value,func,P1)\
	LUA_CLASS_MEMBER_FUNCTION_1(name,return_value,func,P1,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_1_CONST_RENAME(name,return_value,func,P1)\
	LUA_CLASS_MEMBER_FUNCTION_1(name,return_value,func,P1,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_2(return_value,func,P1,P2)\
	LUA_CLASS_MEMBER_FUNCTION_2(func,return_value,func,P1,P2,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_2_CONST(return_value,func,P1,P2)\
	LUA_CLASS_MEMBER_FUNCTION_2(func,return_value,func,P1,P2,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_2_RENAME(name,return_value,func,P1,P2)\
	LUA_CLASS_MEMBER_FUNCTION_2(name,return_value,func,P1,P2,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_2_CONST_RENAME(name,return_value,func,P1,P2)\
	LUA_CLASS_MEMBER_FUNCTION_2(name,return_value,func,P1,P2,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_3(return_value,func,P1,P2,P3)\
	LUA_CLASS_MEMBER_FUNCTION_3(func,return_value,func,P1,P2,P3,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_3_CONST(return_value,func,P1,P2,P3)\
	LUA_CLASS_MEMBER_FUNCTION_3(func,return_value,func,P1,P2,P3,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_3_RENAME(name,return_value,func,P1,P2,P3)\
	LUA_CLASS_MEMBER_FUNCTION_3(name,return_value,func,P1,P2,P3,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_3_CONST_RENAME(name,return_value,func,P1,P2,P3)\
	LUA_CLASS_MEMBER_FUNCTION_3(name,return_value,func,P1,P2,P3,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_4(return_value,func,P1,P2,P3,P4)\
	LUA_CLASS_MEMBER_FUNCTION_4(func,return_value,func,P1,P2,P3,P4,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_4_CONST(return_value,func,P1,P2,P3,P4)\
	LUA_CLASS_MEMBER_FUNCTION_4(func,return_value,func,P1,P2,P3,P4,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_4_RENAME(name,return_value,func,P1,P2,P3,P4)\
	LUA_CLASS_MEMBER_FUNCTION_4(name,return_value,func,P1,P2,P3,P4,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_4_CONST_RENAME(name,return_value,func,P1,P2,P3,P4)\
	LUA_CLASS_MEMBER_FUNCTION_4(name,return_value,func,P1,P2,P3,P4,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_5(return_value,func,P1,P2,P3,P4,P5)\
	LUA_CLASS_MEMBER_FUNCTION_5(func,return_value,func,P1,P2,P3,P4,P5,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_5_CONST(return_value,func,P1,P2,P3,P4,P5)\
	LUA_CLASS_MEMBER_FUNCTION_5(func,return_value,func,P1,P2,P3,P4,P5,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_5_RENAME(name,return_value,func,P1,P2,P3,P4,P5)\
	LUA_CLASS_MEMBER_FUNCTION_5(name,return_value,func,P1,P2,P3,P4,P5,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_5_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5)\
	LUA_CLASS_MEMBER_FUNCTION_5(name,return_value,func,P1,P2,P3,P4,P5,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_6(return_value,func,P1,P2,P3,P4,P5,P6)\
	LUA_CLASS_MEMBER_FUNCTION_6(func,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_6_CONST(return_value,func,P1,P2,P3,P4,P5,P6)\
	LUA_CLASS_MEMBER_FUNCTION_6(func,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_6_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6)\
	LUA_CLASS_MEMBER_FUNCTION_6(name,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_6_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6)\
	LUA_CLASS_MEMBER_FUNCTION_6(name,return_value,func,P1,P2,P3,P4,P5,P6,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_7(return_value,func,P1,P2,P3,P4,P5,P6,P7)\
	LUA_CLASS_MEMBER_FUNCTION_7(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_7_CONST(return_value,func,P1,P2,P3,P4,P5,P6,P7)\
	LUA_CLASS_MEMBER_FUNCTION_7(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_7_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7)\
	LUA_CLASS_MEMBER_FUNCTION_7(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_7_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7)\
	LUA_CLASS_MEMBER_FUNCTION_7(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,OOLUA_CONST_FUNC)


#define OOLUA_MEM_FUNC_8(return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
	LUA_CLASS_MEMBER_FUNCTION_8(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_8_CONST(return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
	LUA_CLASS_MEMBER_FUNCTION_8(func,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_CONST_FUNC)
#define OOLUA_MEM_FUNC_8_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
	LUA_CLASS_MEMBER_FUNCTION_8(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_NON_CONST_FUNC)
#define OOLUA_MEM_FUNC_8_CONST_RENAME(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8)\
	LUA_CLASS_MEMBER_FUNCTION_8(name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,OOLUA_CONST_FUNC)


//proxy implementations
#define LUA_CLASS_MEMBER_FUNCTION_0(func_name,return_value,func,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )()mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<funcType>(l,m_this,&class_::func);\
	return total_out_params< Type_list<out_p<return_value > >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_1(func_name,return_value,func,P1,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_1(P1)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,funcType>(l,m_this,&class_::func,p1);\
	OOLUA_BACK_INTERNAL_1\
	return total_out_params< Type_list<out_p<return_value >,P1_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_2(func_name,return_value,func,P1,P2,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_2(P1,P2)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,funcType>(l,m_this,&class_::func,p1,p2);\
	OOLUA_BACK_INTERNAL_2\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_3(func_name,return_value,func,P1,P2,P3,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_3(P1,P2,P3)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,funcType>(l,m_this,&class_::func,p1,p2,p3);\
	OOLUA_BACK_INTERNAL_3\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_4(func_name,return_value,func,P1,P2,P3,P4,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_4(P1,P2,P3,P4)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type,P4_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,funcType>(l,m_this,&class_::func,p1,p2,p3,p4);\
	OOLUA_BACK_INTERNAL_4\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_5(func_name,return_value,func,P1,P2,P3,P4,P5,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_5(P1,P2,P3,P4,P5)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,funcType>(l,m_this,&class_::func,p1,p2,p3,p4,p5);\
	OOLUA_BACK_INTERNAL_5\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_6(func_name,return_value,func,P1,P2,P3,P4,P5,P6,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_6(P1,P2,P3,P4,P5,P6)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,funcType>(l,m_this,&class_::func,p1,p2,p3,p4,p5,p6);\
	OOLUA_BACK_INTERNAL_6\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_7(func_name,return_value,func,P1,P2,P3,P4,P5,P6,P7,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_7(P1,P2,P3,P4,P5,P6,P7)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_,funcType>(l,m_this,&class_::func,p1,p2,p3,p4,p5,p6,p7);\
	OOLUA_BACK_INTERNAL_7\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_ >::type> ::out;\
}
#define LUA_CLASS_MEMBER_FUNCTION_8(func_name,return_value,func,P1,P2,P3,P4,P5,P6,P7,P8,mod)\
int func_name(lua_State* const l)mod\
{\
	assert(m_this);\
	OOLUA_PARAMS_INTERNAL_8(P1,P2,P3,P4,P5,P6,P7,P8)\
	typedef param_type<return_value > R;\
	typedef R::type (class_::*funcType )(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type,P8_::type)mod ;\
	OOLUA::Proxy_caller<R,class_,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_,funcType>(l,m_this,&class_::func,p1,p2,p3,p4,p5,p6,p7,p8);\
	OOLUA_BACK_INTERNAL_8\
	return total_out_params< Type_list<out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_ >::type> ::out;\
}


#endif 
