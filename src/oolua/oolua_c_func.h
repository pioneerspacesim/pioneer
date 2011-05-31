///////////////////////////////////////////////////////////////////////////////
///  @file oolua_c_func.h
///  @remarks Warning this file was generated, edits to it will not persist if 
///  the file is regenerated.
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence 
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////
#ifndef OOLUA_C_FUNC_H_
#	define OOLUA_C_FUNC_H_

#	include "param_traits.h"
#	include "oolua_paramater_macros.h"
//proxy implementations
#define OOLUA_C_FUNCTION_0(return_value,func) \
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)() ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<funcType>(l,&func);\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value > >::type> ::out;\

#define OOLUA_C_FUNCTION_1(return_value,func,P1) \
	OOLUA_PARAMS_INTERNAL_1(P1)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,funcType>(l,&func,p1);\
	OOLUA_BACK_INTERNAL_1\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_ >::type> ::out;\

#define OOLUA_C_FUNCTION_2(return_value,func,P1,P2) \
	OOLUA_PARAMS_INTERNAL_2(P1,P2)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,funcType>(l,&func,p1,p2);\
	OOLUA_BACK_INTERNAL_2\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_ >::type> ::out;\

#define OOLUA_C_FUNCTION_3(return_value,func,P1,P2,P3) \
	OOLUA_PARAMS_INTERNAL_3(P1,P2,P3)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,funcType>(l,&func,p1,p2,p3);\
	OOLUA_BACK_INTERNAL_3\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_ >::type> ::out;\

#define OOLUA_C_FUNCTION_4(return_value,func,P1,P2,P3,P4) \
	OOLUA_PARAMS_INTERNAL_4(P1,P2,P3,P4)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type,P4_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,funcType>(l,&func,p1,p2,p3,p4);\
	OOLUA_BACK_INTERNAL_4\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_,P4_ >::type> ::out;\

#define OOLUA_C_FUNCTION_5(return_value,func,P1,P2,P3,P4,P5) \
	OOLUA_PARAMS_INTERNAL_5(P1,P2,P3,P4,P5)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,funcType>(l,&func,p1,p2,p3,p4,p5);\
	OOLUA_BACK_INTERNAL_5\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_,P4_,P5_ >::type> ::out;\

#define OOLUA_C_FUNCTION_6(return_value,func,P1,P2,P3,P4,P5,P6) \
	OOLUA_PARAMS_INTERNAL_6(P1,P2,P3,P4,P5,P6)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,funcType>(l,&func,p1,p2,p3,p4,p5,p6);\
	OOLUA_BACK_INTERNAL_6\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_ >::type> ::out;\

#define OOLUA_C_FUNCTION_7(return_value,func,P1,P2,P3,P4,P5,P6,P7) \
	OOLUA_PARAMS_INTERNAL_7(P1,P2,P3,P4,P5,P6,P7)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_,funcType>(l,&func,p1,p2,p3,p4,p5,p6,p7);\
	OOLUA_BACK_INTERNAL_7\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_ >::type> ::out;\

#define OOLUA_C_FUNCTION_8(return_value,func,P1,P2,P3,P4,P5,P6,P7,P8) \
	OOLUA_PARAMS_INTERNAL_8(P1,P2,P3,P4,P5,P6,P7,P8)\
	typedef OOLUA::param_type<return_value > R;\
	typedef R::type (funcType)(P1_::type,P2_::type,P3_::type,P4_::type,P5_::type,P6_::type,P7_::type,P8_::type) ;\
	OOLUA::Proxy_none_member_caller<R,LVD::is_void< R::type >::value >::call<P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_,funcType>(l,&func,p1,p2,p3,p4,p5,p6,p7,p8);\
	OOLUA_BACK_INTERNAL_8\
	return OOLUA::total_out_params< Type_list<OOLUA::out_p<return_value >,P1_,P2_,P3_,P4_,P5_,P6_,P7_,P8_ >::type> ::out;\



#endif 
